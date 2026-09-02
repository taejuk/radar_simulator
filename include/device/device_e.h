#ifndef DEVICE_E_H
#define DEVICE_E_H


#include "common/comm_thread.h"


int device_e_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


#endif