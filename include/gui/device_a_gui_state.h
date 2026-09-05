#ifndef DEVICE_A_GUI_STATE_H
#define DEVICE_A_GUI_STATE_H

#include "common/packet.h"
#include "config.h"

#include <pthread.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct device_a_gui_state
{
    pthread_mutex_t mutex;

    /*
     * GUI가 Send 버튼을 누르면
     * 통신 스레드를 깨운다.
     */
    pthread_cond_t response_condition;


    /*
     * 현재 GUI에 표시할 요청이 있는지
     */
    int request_pending;


    /*
     * GUI가 응답 값을 입력하고
     * Send 버튼을 눌렀는지
     */
    int response_ready;


    int shutdown_requested;


    /*
     * 서버로부터 받은 요청
     */
    packet_header_t request_header;

    uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ];


    /*
     * GUI에서 작성한 응답
     */
    packet_header_t response_header;

    uint8_t response_payload[
        MAX_PAYLOAD_SIZE
    ];

} device_a_gui_state_t;


int device_a_gui_state_init(
    device_a_gui_state_t *state
);


void device_a_gui_state_destroy(
    device_a_gui_state_t *state
);


/*
 * 통신 스레드에서 호출한다.
 *
 * 요청을 GUI에 공개하고,
 * 사용자가 Send를 누를 때까지 기다린다.
 */
int device_a_gui_wait_for_response(
    device_a_gui_state_t *state,
    const packet_header_t *request_header,
    const uint8_t *request_payload,
    packet_header_t *response_header,
    uint8_t *response_payload
);


/*
 * GUI에서 현재 요청을 복사한다.
 *
 * 반환값:
 * 1: 표시할 요청이 있음
 * 0: 요청이 없음
 */
int device_a_gui_get_request(
    device_a_gui_state_t *state,
    packet_header_t *request_header,
    uint8_t *request_payload
);


/*
 * GUI의 Send 버튼에서 호출한다.
 */
int device_a_gui_submit_response(
    device_a_gui_state_t *state,
    uint16_t type,
    uint32_t value,
    uint8_t mode,
    uint8_t status,
    const char *payload
);

/*
 * GUI 종료를 통신 스레드에 알린다.
 */
void device_a_gui_request_shutdown(
    device_a_gui_state_t *state
);

#ifdef __cplusplus
}
#endif

#endif