#ifndef COMM_THREAD_H
#define COMM_THREAD_H

#include "common/packet.h"

#include <stdint.h>


typedef struct device_context
    device_context_t;

typedef struct device_gui_state
    device_gui_state_t;


typedef int (*packet_handler_t)(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload
);


struct device_context
{
    int device_id;

    uint16_t port;

    int is_server;

    int listen_fd;

    int client_fd;

    int state;

    /*
     * Device별로 각각 다른 GUI 상태를 가리킨다.
     */
    device_gui_state_t *gui_state;

    packet_handler_t packet_handler;
};


void *comm_thread(
    void *arg
);


#endif