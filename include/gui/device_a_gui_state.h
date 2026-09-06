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


    /*
     * GUI 상태 플래그
     */
    int request_pending;

    int response_ready;

    int shutdown_requested;


    /*
     * 서버로부터 받은 요청
     *
     * GUI 화면에 받은 요청을 표시하기 위해
     * 반드시 유지해야 한다.
     */
    packet_header_t request_header;

    uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ];


    /*
     * GUI에서 입력한 Device A 응답
     */
    device_a_response_packet_t response_packet;

} device_a_gui_state_t;


/*
 * GUI 공유 상태 초기화
 */
int device_a_gui_state_init(
    device_a_gui_state_t *state
);


/*
 * GUI 공유 상태 해제
 */
void device_a_gui_state_destroy(
    device_a_gui_state_t *state
);


/*
 * 통신 스레드에서 호출한다.
 *
 * 요청을 GUI에 전달하고 사용자가 Send 버튼을
 * 누를 때까지 기다린다.
 */
int device_a_gui_wait_for_response(
    device_a_gui_state_t *state,
    const packet_header_t *request_header,
    const uint8_t *request_payload,
    device_a_response_packet_t *response_packet
);


/*
 * GUI가 현재 수신 요청을 가져온다.
 *
 * 반환값:
 *  1: 요청 있음
 *  0: 요청 없음
 * -1: 오류
 */
int device_a_gui_get_request(
    device_a_gui_state_t *state,
    packet_header_t *request_header,
    uint8_t *request_payload
);


/*
 * GUI에서 Send 버튼을 눌렀을 때 호출한다.
 */
int device_a_gui_submit_response(
    device_a_gui_state_t *state,
    const device_a_response_packet_t *response_packet
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