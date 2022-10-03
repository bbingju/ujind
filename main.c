#define _GNU_SOURCE

#include "broadcast.h"

#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <curl/curl.h>


static struct _ujin_context {
    struct mosquitto *mosq;
} ujin_context;



static int _get_file(const char *url, const char *filename)
{
    if (!url || !filename)
        return -1;

    CURL *curl = curl_easy_init();
    if (!curl) {
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

    fclose(file);
    curl_easy_cleanup(curl);

    return res != CURLE_OK ? -1 : 0;
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    int rc;

    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
	mosquitto_disconnect(mosq);
    }

    rc = mosquitto_subscribe(mosq, NULL, "ujin/v1/broadcast/start/1", 1);
    if (rc != MOSQ_ERR_SUCCESS) {
	fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
	mosquitto_disconnect(mosq);
    }
}


void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    bool have_subscription = false;

    for (i = 0; i < qos_count; i++){
	printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
	if (granted_qos[i] <= 2){
	    have_subscription = true;
	}
    }
    if (have_subscription == false) {
	fprintf(stderr, "Error: All subscriptions rejected.\n");
	mosquitto_disconnect(mosq);
    }
}


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    const char* BASE_URL = "http://192.168.0.65:55580/api/Broadcast/Download";
    char *url = NULL;
    char *fn = NULL;

    printf("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);

    asprintf(&url, "%s?id=%s", BASE_URL, (char *)msg->payload);
    asprintf(&fn, "/tmp/%s.mp3", (char *)msg->payload);

    if (_get_file(url, fn) == 0)
        broadcast(fn);

    if (url)
	free(url);

    if (fn)
	free(fn);
}


int main(int argc, char *argv[])
{
    int rc;

    mosquitto_lib_init();

    if ((ujin_context.mosq = mosquitto_new(NULL, true, NULL)) == NULL) {
	perror("mosquitto_new error.");
	mosquitto_lib_cleanup();
	return -1;
    }

    mosquitto_connect_callback_set(ujin_context.mosq, on_connect);
    mosquitto_subscribe_callback_set(ujin_context.mosq, on_subscribe);
    mosquitto_message_callback_set(ujin_context.mosq, on_message);

    rc = mosquitto_connect(ujin_context.mosq, "test.mosquitto.org", 1883, 60);
    if(rc != MOSQ_ERR_SUCCESS){
	mosquitto_destroy(ujin_context.mosq);
	fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	return 1;
    }

    mosquitto_loop_forever(ujin_context.mosq, -1, 1);

    mosquitto_lib_cleanup();

    return 0;
}
