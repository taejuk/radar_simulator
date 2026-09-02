#ifndef DEVICE_B_H
#define DEVICE_B_H


#include "common/comm_thread.h"


int device_b_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


#endif