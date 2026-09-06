#ifndef DEVICE_A_H
#define DEVICE_A_H

#include "common/comm_thread.h"


int device_a_handler(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload
);


#endif