#ifndef MCU_CMD_H
#define MCU_CMD_H

#include "context.h"

typedef enum {
    MCU_CMD_AUDIOSEL,
} mcu_cmd_type_e;

typedef enum {
  AUDIOSEL_MIC,
  AUDIOSEL_OUT,
} audiosel_type_e;

typedef struct {
    mcu_cmd_type_e type;
    union {
	audiosel_type_e audiosel;
    } u;
} mcu_cmd_t;

typedef struct {
  int result;
} mcu_cmd_received_t;

typedef void (*mcu_cmd__receive_cb_f)(struct context *ctx, mcu_cmd_received_t* result);

int mcu_cmd__open(struct context *ctx, mcu_cmd__receive_cb_f cb);
void mcu_cmd__close(struct context *ctx);

int mcu_cmd__send(struct context *ctx, mcu_cmd_t *cmd);


#endif /* MCU_CMD_H */
