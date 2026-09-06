#ifndef DEVICE_A_GUI_STATE_H
#define DEVICE_A_GUI_STATE_H

#include "common/packet.h"
#include "device/device_a_packet.h"

#include "config.h"

#include <pthread.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct device_a_gui_state
{
    pthread_mutex_t mutex;

    pthread_cond_t response_condition;

    int request_pending;

    int response_ready;

    int shutdown_requested;


    /*
     * 수신 Header는 Host Endian 상태로 저장된다.
     */
    InternalMsgHeader_t request_header;

    uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ];


    /*
     * GUI에서 입력하는 payload 구조체
     */
    device_a_response_packet_t response_packet;

} device_a_gui_state_t;


int device_a_gui_state_init(
    device_a_gui_state_t *state
);


void device_a_gui_state_destroy(
    device_a_gui_state_t *state
);


int device_a_gui_wait_for_response(
    device_a_gui_state_t *state,
    const InternalMsgHeader_t *request_header,
    const uint8_t *request_payload,
    device_a_response_packet_t *response_packet
);


int device_a_gui_get_request(
    device_a_gui_state_t *state,
    InternalMsgHeader_t *request_header,
    uint8_t *request_payload
);


int device_a_gui_submit_response(
    device_a_gui_state_t *state,
    const device_a_response_packet_t *response_packet
);


void device_a_gui_request_shutdown(
    device_a_gui_state_t *state
);


#ifdef __cplusplus
}
#endif


#endif