#include "lte_at.h"
#include "conf.h"

#define LOG_MODULE_NAME "lte_at"
#define MAX_LOG_LEVEL DEBUG_LOG_LEVEL
#include "logger.h"

#include <string.h>

static lte_at__receive_cb_f _on_received_cb;

static void _default_on_received(struct context *ctx, char *key, char *value)
{
    LOGD("key: %s, value: %s\n", key, value);
}

int lte_at__open(struct context *ctx, lte_at__receive_cb_f cb)
{
    ctx->lte_at = serial_open(conf__lte_modem_serial_port(),
			      conf__lte_modem_serial_baudrate());
    if (!ctx->lte_at) {
	return -1;
    }

    _on_received_cb = cb ? cb : _default_on_received;

    return 0;
}

void lte_at__close(struct context *ctx)
{
    serial_close(ctx->lte_at);
}

int lte_at__send(struct context *ctx, const char* cmd)
{
    char readbuf[1024] = { 0 };
    char *str1, *str2, *token, *subtoken;
    char *saveptr1, *saveptr2;

    if (!ctx->lte_at) {
	return -1;
    }

    int ret = serial_write(ctx->lte_at, (const unsigned char *) cmd, strlen(cmd));
    if (ret < 0) {
	LOGE("serial_write error\n");
	return -1;
    }
    ret = serial_write(ctx->lte_at, "\r\n", 2);
    if (ret < 0) {
	LOGE("Serial_write error\n");
	return -1;
    }

    unsigned char c;
    int i = 0;
    do {
	ret = serial_read(ctx->lte_at, &c, 300);
	if (ret < 0) {
	    LOGE("Serial_read error\n");
	    return -1;
	}
	readbuf[i++] = c;

    } while (ret > 0);

    LOGD("%s", readbuf);

    for (int j = 1, str1 = readbuf; ; j++, str1 = NULL) {
	token = strtok_r(str1, "\r\n", &saveptr1);
	if (token == NULL)
	    break;
	/* printf("%d: %s\n", j, token); */

	char *key = NULL;
	for (str2 = token; ; str2 = NULL) {
	    subtoken = strtok_r(str2, ":", &saveptr2);
	    if (subtoken == NULL)
		break;
	    /* printf(" --> %s\n", subtoken); */

	    if (!key)
		key = subtoken;
	    else {
		_on_received_cb(ctx, key, subtoken);
		key = NULL;
	    }
	}
    }

    return i;
}
