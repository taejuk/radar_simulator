#ifndef TEST_EXCHANGE_COMMON_H
#define TEST_EXCHANGE_COMMON_H

#include <stdint.h>


typedef void (*test_response_printer_t)(
    const void *response_packet
);


int test_run_client(
    const char *device_name,
    uint16_t device_port,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer
);


int test_run_server(
    const char *device_name,
    uint16_t device_port,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer
);


#endif