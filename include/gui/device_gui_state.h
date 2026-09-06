#ifndef DEVICE_GUI_STATE_H
#define DEVICE_GUI_STATE_H

#include "common/packet.h"
#include "config.h"

#include <pthread.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * 특정 Device payload 타입에 의존하지 않는
 * 공통 GUI 공유 상태다.
 *
 * payload는 byte buffer로 저장하기 때문에
 * Device별로 서로 다른 구조체를 사용할 수 있다.
 */
typedef struct device_gui_state
{
    pthread_mutex_t mutex;

    pthread_cond_t response_condition;

    int request_pending;

    int response_ready;

    int shutdown_requested;

    /*
     * 동일한 Header가 연속으로 들어와도
     * 새로운 요청을 구분하기 위한 번호다.
     */
    uint64_t request_generation;


    /*
     * 수신 요청
     */
    InternalMsgHeader_t request_header;

    uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ];


    /*
     * GUI에서 생성한 응답
     */
    uint16_t response_msg_type;

    uint32_t response_payload_size;

    uint8_t response_payload[
        MAX_PAYLOAD_SIZE
    ];

} device_gui_state_t;


int device_gui_state_init(
    device_gui_state_t *state
);


void device_gui_state_destroy(
    device_gui_state_t *state
);


int device_gui_wait_for_response(
    device_gui_state_t *state,
    const InternalMsgHeader_t *request_header,
    const uint8_t *request_payload,
    uint16_t *response_msg_type,
    uint8_t *response_payload,
    uint32_t response_capacity,
    uint32_t *response_payload_size
);


int device_gui_get_request(
    device_gui_state_t *state,
    InternalMsgHeader_t *request_header,
    uint8_t *request_payload,
    uint64_t *request_generation
);


int device_gui_submit_response(
    device_gui_state_t *state,
    uint16_t response_msg_type,
    const void *response_payload,
    uint32_t response_payload_size
);


void device_gui_request_shutdown(
    device_gui_state_t *state
);


#ifdef __cplusplus
}
#endif


#endif