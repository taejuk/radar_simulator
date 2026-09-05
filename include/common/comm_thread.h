#ifndef COMM_THREAD_H
#define COMM_THREAD_H


#include <stdint.h>

#include "common/packet.h"


typedef struct device_context device_context_t;

typedef struct device_a_gui_state
    device_a_gui_state_t;

/*
 * 장비별 packet handler 함수 형태
 */
typedef int (*packet_handler_t)(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload
);


struct device_context
{
    /*
     * A/B/C... 장비 식별용
     */
    int device_id;


    /*
     * 해당 장비가 사용하는 TCP port
     */
    uint16_t port;

    int is_server;
    /*
     * TCP file descriptor
     */
    int listen_fd;

    int client_fd;


    /*
     * 시뮬레이터 장비 내부 상태
     */
    int state;


    /*
     * 장비별 packet 처리 함수
     */
    device_a_gui_state_t *gui_state;
    packet_handler_t packet_handler;
};


void *comm_thread(
    void *arg
);


#endif