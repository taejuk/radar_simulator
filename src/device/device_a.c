#include "device/device_a.h"

#include "handler/device_a_handler.h"


int device_a_handler(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    if (header == NULL)
    {
        return -1;
    }


    switch (header->msgType)
    {
        case PACKET_START:
            return device_a_handle_start(
                ctx,
                header,
                payload
            );

        case PACKET_STATUS:
            return device_a_handle_status(
                ctx,
                header,
                payload
            );

        case PACKET_CONTROL:
            return device_a_handle_control(
                ctx,
                header,
                payload
            );

        default:
            return -1;
    }
}