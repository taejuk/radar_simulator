#include "gui/device_a_gui_state.h"

#include <stddef.h>
#include <string.h>



int device_a_gui_state_init(
    device_a_gui_state_t *state)
{
    int ret;


    if (state == NULL)
    {
        return -1;
    }


    /*
     * flag, header, payload를 모두 0으로 초기화한다.
     */
    memset(
        state,
        0,
        sizeof(*state)
    );


    ret = pthread_mutex_init(
        &state->mutex,
        NULL
    );

    if (ret != 0)
    {
        return -1;
    }


    ret = pthread_cond_init(
        &state->response_condition,
        NULL
    );

    if (ret != 0)
    {
        pthread_mutex_destroy(
            &state->mutex
        );

        return -1;
    }


    return 0;
}


void device_a_gui_state_destroy(
    device_a_gui_state_t *state)
{
    if (state == NULL)
    {
        return;
    }


    /*
     * 이 함수는 통신 스레드가 종료된 다음 호출해야 한다.
     */
    pthread_cond_destroy(
        &state->response_condition
    );

    pthread_mutex_destroy(
        &state->mutex
    );
}

int device_a_gui_wait_for_response(
    device_a_gui_state_t *state,
    const packet_header_t *request_header,
    const uint8_t *request_payload,
    device_a_response_packet_t *response_packet)
{
    int ret;


    if ((state == NULL) ||
        (request_header == NULL) ||
        (response_packet == NULL))
    {
        return -1;
    }


    if (request_header->length >
        MAX_PAYLOAD_SIZE)
    {
        return -1;
    }


    if ((request_header->length > 0U) &&
        (request_payload == NULL))
    {
        return -1;
    }


    ret = pthread_mutex_lock(
        &state->mutex
    );

    if (ret != 0)
    {
        return -1;
    }


    if (state->shutdown_requested != 0)
    {
        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    /*
     * 받은 요청을 GUI 공유 상태에 저장한다.
     */
    state->request_header =
        *request_header;


    if (request_header->length > 0U)
    {
        memcpy(
            state->request_payload,
            request_payload,
            request_header->length
        );
    }


    state->request_payload[
        request_header->length
    ] = '\0';


    state->request_pending = 1;
    state->response_ready = 0;


    /*
     * GUI에서 Send 버튼을 누를 때까지 기다린다.
     */
    while ((state->response_ready == 0) &&
           (state->shutdown_requested == 0))
    {
        ret = pthread_cond_wait(
            &state->response_condition,
            &state->mutex
        );

        if (ret != 0)
        {
            pthread_mutex_unlock(
                &state->mutex
            );

            return -1;
        }
    }


    if (state->shutdown_requested != 0)
    {
        state->request_pending = 0;

        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    /*
     * GUI에서 입력한 응답 구조체를
     * 통신 스레드의 지역 변수로 복사한다.
     */
    *response_packet =
        state->response_packet;


    state->request_pending = 0;
    state->response_ready = 0;


    pthread_mutex_unlock(
        &state->mutex
    );


    return 0;
}



int device_a_gui_get_request(
    device_a_gui_state_t *state,
    packet_header_t *request_header,
    uint8_t *request_payload)
{
    int ret;
    int has_request;


    if ((state == NULL) ||
        (request_header == NULL) ||
        (request_payload == NULL))
    {
        return -1;
    }


    ret = pthread_mutex_lock(
        &state->mutex
    );

    if (ret != 0)
    {
        return -1;
    }


    has_request =
        state->request_pending;


    if (has_request != 0)
    {
        *request_header =
            state->request_header;


        /*
         * 마지막 NULL 문자까지 복사한다.
         */
        memcpy(
            request_payload,
            state->request_payload,
            state->request_header.length + 1U
        );
    }
    else
    {
        /*
         * 이전 요청 정보가 GUI에 남지 않도록 초기화한다.
         */
        memset(
            request_header,
            0,
            sizeof(*request_header)
        );

        request_payload[0] = '\0';
    }


    pthread_mutex_unlock(
        &state->mutex
    );


    return has_request;
}

int device_a_gui_submit_response(
    device_a_gui_state_t *state,
    const device_a_response_packet_t *response_packet)
{
    int ret;


    if ((state == NULL) ||
        (response_packet == NULL))
    {
        return -1;
    }


    ret = pthread_mutex_lock(
        &state->mutex
    );

    if (ret != 0)
    {
        return -1;
    }


    /*
     * 요청이 없거나 이미 응답을 제출했으면
     * 중복 응답을 허용하지 않는다.
     */
    if ((state->request_pending == 0) ||
        (state->response_ready != 0) ||
        (state->shutdown_requested != 0))
    {
        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    /*
     * GUI에서 편집한 패킷의 스냅샷을 저장한다.
     */
    state->response_packet =
        *response_packet;


    state->response_ready = 1;


    pthread_cond_signal(
        &state->response_condition
    );


    pthread_mutex_unlock(
        &state->mutex
    );


    return 0;
}


void device_a_gui_request_shutdown(
    device_a_gui_state_t *state)
{
    int ret;


    if (state == NULL)
    {
        return;
    }


    ret = pthread_mutex_lock(
        &state->mutex
    );

    if (ret != 0)
    {
        return;
    }


    state->shutdown_requested = 1;


    /*
     * GUI 응답을 기다리는 통신 스레드가 있다면
     * 종료할 수 있도록 깨운다.
     */
    pthread_cond_broadcast(
        &state->response_condition
    );


    pthread_mutex_unlock(
        &state->mutex
    );
}