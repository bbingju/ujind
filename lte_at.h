#ifndef LTE_AT_H
#define LTE_AT_H

#include "context.h"

typedef void (*lte_at__receive_cb_f)(struct context *ctx, char *key, char *value);

int lte_at__open(struct context *ctx, lte_at__receive_cb_f cb);
void lte_at__close(struct context *ctx);
int lte_at__send(struct context *ctx, const char* cmd);

#endif /* LTE_AT_H */
