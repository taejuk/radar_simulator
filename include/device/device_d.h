#ifndef DEVICE_D_H
#define DEVICE_D_H


#include "common/comm_thread.h"


int device_d_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


#endif