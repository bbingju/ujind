#define _GNU_SOURCE

#define USE_LTE_MODEM 1

#include "context.h"
#include "conf.h"
#if USE_LTE_MODEM
#include "lte_at.h"
#endif
#include "mcu_cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mosquitto.h>
#include <curl/curl.h>

#define LOG_MODULE_NAME "ujin"
#define MAX_LOG_LEVEL DEBUG_LOG_LEVEL
#include "logger.h"


struct context context;

static int _get_file(const char *url, const char *filename)
{
    if (!url || !filename) {
	fprintf(stderr, "error: Invalid args\n");
	return -1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
	fprintf(stderr, "curl_easy_init error\n");
	return -1;
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
	perror("fopen");
	curl_easy_cleanup(curl);
	return -1;
    }

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "http");

    struct curl_slist *headers = NULL;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
	LOGE("%s\n", curl_easy_strerror(res));
    }

    fclose(file);
    curl_easy_cleanup(curl);

    return res != CURLE_OK ? -1 : 0;
}

static char *_get_request_topic(struct context* ctx)
{
#if USE_LTE_MODEM
    if (!ctx->lte_modem_number) {
	lte_at__send(ctx, "AT$$SHORT_NUM?");
	sleep(1);
    }
#endif

    if (!ctx->lte_modem_number) {
	ctx->lte_modem_number = conf__lte_modem_fake_number();
    }

    if (!ctx->req_topic) {
	asprintf(&ctx->req_topic, "ujin-req/%s", ctx->lte_modem_number);
    }

    return ctx->req_topic;
}

static char *_get_response_topic(struct context* ctx)
{
#if USE_LTE_MODEM
    if (!ctx->lte_modem_number) {
	lte_at__send(ctx, "AT$$SHORT_NUM?");
	sleep(1);
    }
#endif

    if (!ctx->lte_modem_number) {
	ctx->lte_modem_number = conf__lte_modem_fake_number();
    }

    if (!ctx->res_topic) {
	asprintf(&ctx->res_topic, "ujin-res/%s", ctx->lte_modem_number);
    }

    return ctx->res_topic;
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    struct context *ctx = obj;
    int rc;

    LOGI("%s\n", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
	mosquitto_disconnect(mosq);
    }

    rc = mosquitto_subscribe(mosq, NULL, _get_request_topic(ctx), 1);
    if (rc != MOSQ_ERR_SUCCESS) {
	LOGE("Error subscribing: %s\n", mosquitto_strerror(rc));
	mosquitto_disconnect(mosq);
    }
}


void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    bool have_subscription = false;

    for (i = 0; i < qos_count; i++){
	LOGI("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
	if (granted_qos[i] <= 2){
	    have_subscription = true;
	}
    }
    if (have_subscription == false) {
	LOGE("error: All subscriptions rejected.\n");
	mosquitto_disconnect(mosq);
    }
}

pid_t child_player = -1;

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    struct context *ctx = obj;
    char *url = NULL;
    int ret;
    char* fn = "/tmp/bc.mp3";

    LOGD("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);

    if (strlen((char *)msg->payload) < 1) {
	LOGW("No data in the payload\n");
	return;
    }

    ret = asprintf(&url,
		   "http://%s:%d/api/Broadcast/Download?id=%s",
		   conf__server_host(), conf__server_port(), (char *)msg->payload);
    if (ret == -1) {
	perror("asprintf");
	goto _ret;
    }

    if (_get_file(url, fn) < 0) {
	goto _ret;
    }

    /* Send a command to the mcu for selecting audio path */
    mcu_cmd_t cmd = {.type = MCU_CMD_AUDIOSEL,
                     .u = { .audiosel = AUDIOSEL_OUT, }};
    mcu_cmd__send(&context, &cmd);

    switch (child_player = fork()) {
    case -1:
	LOGE("forking error\n");
	goto _ret;

    case 0:	{		/* child */
	char *newargv[] = { "/usr/bin/mpg123", NULL, NULL };
	char *newenviron[] = { NULL };
	newargv[1] = fn;
	execve(newargv[0], newargv, newenviron);
	perror("execve");
	break;
    }

    default: {
	char retval[128] = { 0 };
	sprintf(retval, "%d", child_player);
	int rc = mosquitto_publish(ctx->mosq, NULL, _get_response_topic(ctx),
				   strlen(retval) + 1, retval, 0, false);
	if (rc != MOSQ_ERR_SUCCESS) {
	    LOGE("%s\n", mosquitto_strerror(rc));
	}
	break;
    }
    }

_ret:
    if (url)
	free(url);
}


void _on_received_lte_at(struct context *ctx, char *key, char *value)
{
    LOGD("key: %s, value: %s\n", key, value);

    if (strcmp(key, "$$SHORT_NUM") == 0) {
	if (ctx->lte_modem_number)
	    free(ctx->lte_modem_number);
	ctx->lte_modem_number = strdup(value);
	LOGI("Modem real number = %s\n", ctx->lte_modem_number);
    }
}


int main(int argc, char *argv[])
{
    if (argc > 1) {
	LOGI("Delay %d sec before starting.\n", atoi(argv[1]));
        sleep(atoi(argv[1]));
    }

    if (mcu_cmd__open(&context, NULL) < 0) {
	fprintf(stderr, "mcu_cmd__open error\n");
	exit(EXIT_FAILURE);
    }

#if USE_LTE_MODEM
        if (lte_at__open(&context, _on_received_lte_at) < 0) {
	exit(EXIT_FAILURE);
    }
    lte_at__send(&context, "AT$$SHORT_NUM?");
#endif

    mosquitto_lib_init();

    if ((context.mosq = mosquitto_new(NULL, true, &context)) == NULL) {
	perror("mosquitto_new error.");
	mosquitto_lib_cleanup();
	exit(EXIT_FAILURE);
    }

    mosquitto_connect_callback_set(context.mosq, on_connect);
    mosquitto_subscribe_callback_set(context.mosq, on_subscribe);
    mosquitto_message_callback_set(context.mosq, on_message);

    int rc = mosquitto_connect(context.mosq, conf__mqtt_broker_host(), 1883, 60);
    if(rc != MOSQ_ERR_SUCCESS){
	mosquitto_destroy(context.mosq);
	LOGE("mosquitto_connect error: %s\n", mosquitto_strerror(rc));
	exit(EXIT_FAILURE);
    }

    mosquitto_loop_forever(context.mosq, -1, 1);

    mcu_cmd__close(&context);

#if USE_LTE_MODEM
    lte_at__close(&context);
#endif
    mosquitto_lib_cleanup();

    exit(EXIT_SUCCESS);
}
