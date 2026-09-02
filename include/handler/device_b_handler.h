#ifndef DEVICE_B_HANDLER_H
#define DEVICE_B_HANDLER_H

#include "common/comm_thread.h"

int device_b_handle_start(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);

int device_b_handle_status(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);

int device_b_handle_control(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);

#endif