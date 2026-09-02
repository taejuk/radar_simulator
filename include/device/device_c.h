#ifndef DEVICE_C_H
#define DEVICE_C_H


#include "common/comm_thread.h"


int device_c_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


#endif