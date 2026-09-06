#ifndef DEVICE_A_PACKET_H
#define DEVICE_A_PACKET_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    uint16_t message_type;

    uint32_t time_sec;

    uint32_t time_nsec;

    uint32_t radar_status;

    uint8_t mode;

    uint8_t status;

} device_a_response_packet_t;


#ifdef __cplusplus
}
#endif


#endif