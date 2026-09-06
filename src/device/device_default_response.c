#include "device/device_default_response.h"

#include "common/packet.h"
#include "config.h"

#include "device/device_a_packet.h"
#include "device/device_b_packet.h"
#include "device/device_c_packet.h"
#include "device/device_d_packet.h"
#include "device/device_e_packet.h"
#include "device/device_f_packet.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>


typedef union device_default_payload
{
    device_a_response_packet_t device_a;
    device_b_response_packet_t device_b;
    device_c_response_packet_t device_c;
    device_d_response_packet_t device_d;
    device_e_response_packet_t device_e;
    device_f_response_packet_t device_f;
} device_default_payload_t;


_Static_assert(
    sizeof(device_default_payload_t) <= MAX_PAYLOAD_SIZE,
    "A default payload exceeds MAX_PAYLOAD_SIZE"
);


int device_default_response_build(
    int device_id,
    uint16_t *response_msg_type,
    uint8_t *response_payload,
    uint32_t response_capacity,
    uint32_t *response_payload_size)
{
    device_default_payload_t payload;

    size_t payload_size;


    if ((response_msg_type == NULL) ||
        (response_payload == NULL) ||
        (response_payload_size == NULL))
    {
        return -1;
    }


    /*
     * 기본값:
     *
     * message_type = PACKET_STATUS
     * time_sec     = 0
     * time_nsec    = 0
     * radar_status = 0
     * mode         = 1
     * status       = 0
     */
    memset(
        &payload,
        0,
        sizeof(payload)
    );


    switch (device_id)
    {
        case 1:
            payload.device_a.message_type =
                PACKET_STATUS;

            payload.device_a.mode =
                1U;

            payload_size =
                sizeof(payload.device_a);
            break;


        case 2:
            payload.device_b.message_type =
                PACKET_STATUS;

            payload.device_b.mode =
                1U;

            payload_size =
                sizeof(payload.device_b);
            break;


        case 3:
            payload.device_c.message_type =
                PACKET_STATUS;

            payload.device_c.mode =
                1U;

            payload_size =
                sizeof(payload.device_c);
            break;


        case 4:
            payload.device_d.message_type =
                PACKET_STATUS;

            payload.device_d.mode =
                1U;

            payload_size =
                sizeof(payload.device_d);
            break;


        case 5:
            payload.device_e.message_type =
                PACKET_STATUS;

            payload.device_e.mode =
                1U;

            payload_size =
                sizeof(payload.device_e);
            break;


        case 6:
            payload.device_f.message_type =
                PACKET_STATUS;

            payload.device_f.mode =
                1U;

            payload_size =
                sizeof(payload.device_f);
            break;


        default:
            return -1;
    }


    if (payload_size > response_capacity)
    {
        return -1;
    }


    memcpy(
        response_payload,
        &payload,
        payload_size
    );


    *response_msg_type =
        PACKET_STATUS;

    *response_payload_size =
        (uint32_t)payload_size;


    return 0;
}