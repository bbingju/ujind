#ifndef CONTEXT_H
#define CONTEXT_H

#include <mosquitto.h>
#include "serial.h"

struct context {
    struct mosquitto *mosq;
    serial_t *lte_at;
    char *lte_modem_number;
    char *req_topic;
    char *res_topic;
};

extern struct context context;

#endif /* CONTEXT_H */
