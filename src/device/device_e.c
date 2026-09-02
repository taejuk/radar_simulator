#include "device/device_e.h"
#include "handler/device_e_handler.h"

int device_e_handler(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    switch (header->type)
    {
        case PACKET_START:
            return device_e_handle_start(
                ctx,
                header,
                payload
            );

        case PACKET_STATUS:
            return device_e_handle_status(
                ctx,
                header,
                payload
            );

        case PACKET_CONTROL:
            return device_e_handle_control(
                ctx,
                header,
                payload
            );

        default:
            return -1;
    }
}