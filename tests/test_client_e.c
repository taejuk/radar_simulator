#include "test_exchange_common.h"

#include "config.h"
#include "device/device_e_packet.h"

#include <stdint.h>
#include <stdio.h>


static void print_response(
    const void *response_data)
{
    const device_e_response_packet_t *response =
        (const device_e_response_packet_t *)
            response_data;


    printf(
        "[Device E Payload]\n"
        "  message_type = %u\n"
        "  time_sec     = %u\n"
        "  time_nsec    = %u\n"
        "  radar_status = %u\n"
        "  mode         = %u\n"
        "  status       = %u\n",
        (unsigned int)response->message_type,
        (unsigned int)response->time_sec,
        (unsigned int)response->time_nsec,
        (unsigned int)response->radar_status,
        (unsigned int)response->mode,
        (unsigned int)response->status
    );
}


int main(void)
{
    device_e_response_packet_t
        response_packet;


    return test_run_client(
        "E",
        DEVICE_E_PORT,
        5U,
        &response_packet,
        (uint32_t)sizeof(response_packet),
        print_response
    );
}