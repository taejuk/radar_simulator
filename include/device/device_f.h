#ifndef DEVICE_F_H
#define DEVICE_F_H


#include "common/comm_thread.h"


int device_f_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


#endif