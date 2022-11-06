#include "mcu_cmd.h"
#include "conf.h"

#define LOG_MODULE_NAME "mcu_cmd"
#define MAX_LOG_LEVEL DEBUG_LOG_LEVEL
#include "logger.h"

#include <string.h>
#include "cJSON.h"


static mcu_cmd__receive_cb_f _on_received_cb;

static void _default_on_received(struct context *ctx, char *key, char *value)
{
    LOGD("key: %s, value: %s\n", key, value);
}

int mcu_cmd__open(struct context *ctx, mcu_cmd__receive_cb_f cb)
{
    ctx->mcu_cmd = serial_open(conf__audiosel_serial_port(),
                               conf__audiosel_serial_baudrate());
    if (!ctx->mcu_cmd) {
        return -1;
    }

    /* _on_received_cb = cb ? cb : _default_on_received; */

    return 0;
}

void mcu_cmd__close(struct context *ctx)
{
    serial_close(ctx->mcu_cmd);
}

static int _serialize_cmd(const mcu_cmd_t *cmd, char *buffer, size_t length)
{
    cJSON_bool result = cJSON_False;

    if (!cmd || !buffer) {
	fprintf(stderr, "%s: wrong arguments\n", __func__);
	return -1;
    }

    cJSON *root_obj = cJSON_CreateObject();
    if (!root_obj) {
	fprintf(stderr, "%s: cJSON_CreateObject error\n", __func__);
	return -1;
    }

    switch (cmd->type) {
    case MCU_CMD_AUDIOSEL: {
	cJSON_bool val = cmd->u.audiosel == AUDIOSEL_OUT ? cJSON_True : cJSON_False;
	cJSON_AddItemToObject(root_obj, "method", cJSON_CreateString("audiosel"));
	cJSON_AddItemToObject(root_obj, "params", cJSON_CreateBool(val));
	break;
    } 
    default:
	fprintf(stderr, "%s: wrong cmd type\n", __func__);
	goto _ret;
    }

    result = cJSON_PrintPreallocated(root_obj, buffer, length, cJSON_True);
    cJSON_Minify(buffer);
_ret:
    cJSON_Delete(root_obj);
    return result ? 0 : -1;
}

int mcu_cmd__send(struct context *ctx, mcu_cmd_t *cmd)
{
    char buffer[1024] = { 0 };

    if (!ctx->mcu_cmd) {
        return -1;
    }

    int ret = _serialize_cmd(cmd, buffer, 1023);
    if (ret < 0) {
        fprintf(stderr, "serialize_cmd error\n");
	return -1;
    }

    printf("%s: %s\n", __func__, buffer);

    ret = serial_write(ctx->mcu_cmd, (const unsigned char *) buffer, strlen(buffer));
    if (ret < 0) {
        fprintf(stderr, "serial_write error\n");
        return -1;
    }

    return 0;
}
