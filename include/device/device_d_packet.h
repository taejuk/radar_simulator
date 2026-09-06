#ifndef DEVICE_D_PACKET_H
#define DEVICE_D_PACKET_H

#include <stdint.h>


/*
 * Device D 응답 Payload
 */
typedef struct
{
    uint16_t message_type;
    uint32_t time_sec;
    uint32_t time_nsec;
    uint32_t radar_status;
    uint8_t mode;
    uint8_t status;

} device_d_response_packet_t;


#endif