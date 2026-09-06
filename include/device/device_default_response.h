#ifndef DEVICE_DEFAULT_RESPONSE_H
#define DEVICE_DEFAULT_RESPONSE_H

#include <stdint.h>


int device_default_response_build(
    int device_id,
    uint16_t *response_msg_type,
    uint8_t *response_payload,
    uint32_t response_capacity,
    uint32_t *response_payload_size
);


#endif