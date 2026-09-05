#include "gui/device_a_gui_state.h"

#include <stddef.h>
#include <string.h>


/*
 * 최대 길이를 초과하지 않도록 문자열 길이를 계산한다.
 */
static size_t bounded_string_length(
    const char *text,
    size_t limit)
{
    size_t length = 0U;


    while ((length < limit) &&
           (text[length] != '\0'))
    {
        ++length;
    }


    return length;
}


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
    packet_header_t *response_header,
    uint8_t *response_payload)
{
    int ret;


    if ((state == NULL) ||
        (request_header == NULL) ||
        (response_header == NULL) ||
        (response_payload == NULL))
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
     * 통신 스레드가 받은 요청을
     * GUI 공유 상태에 저장한다.
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


    /*
     * GUI에서 문자열로 출력할 수 있도록
     * 마지막에 NULL 문자를 추가한다.
     *
     * request_payload 배열은
     * MAX_PAYLOAD_SIZE + 1 크기이다.
     */
    state->request_payload[
        request_header->length
    ] = '\0';


    state->request_pending = 1;
    state->response_ready = 0;


    /*
     * GUI에서 Send 버튼을 누를 때까지 기다린다.
     *
     * pthread_cond_wait()는 대기하는 동안 mutex를 풀고,
     * 깨어날 때 다시 mutex를 획득한다.
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


    /*
     * GUI 창이 닫힌 경우에는
     * 응답을 전송하지 않고 종료한다.
     */
    if (state->shutdown_requested != 0)
    {
        state->request_pending = 0;

        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    if (state->response_header.length >
        MAX_PAYLOAD_SIZE)
    {
        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    /*
     * GUI가 작성한 응답을
     * 통신 스레드의 지역 변수로 복사한다.
     */
    *response_header =
        state->response_header;


    if (response_header->length > 0U)
    {
        memcpy(
            response_payload,
            state->response_payload,
            response_header->length
        );
    }


    /*
     * 하나의 요청 처리가 완료되었다.
     */
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
    uint16_t type,
    uint32_t value,
    uint8_t mode,
    uint8_t status,
    const char *payload)
{
    size_t payload_length;

    int ret;


    if ((state == NULL) ||
        (payload == NULL))
    {
        return -1;
    }


    /*
     * 현재 정의된 packet type만 허용한다.
     */
    if ((type != PACKET_START) &&
        (type != PACKET_STATUS) &&
        (type != PACKET_CONTROL))
    {
        return -1;
    }


    payload_length =
        bounded_string_length(
            payload,
            MAX_PAYLOAD_SIZE
        );


    /*
     * response_payload 배열의 마지막에는
     * 문자열 종료 문자가 있어야 하므로
     * 최대 전송 길이는 MAX_PAYLOAD_SIZE - 1이다.
     */
    if (payload_length >=
        MAX_PAYLOAD_SIZE)
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
     * 요청이 없거나 이미 응답을 제출한 경우에는
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


    memset(
        &state->response_header,
        0,
        sizeof(state->response_header)
    );


    state->response_header.type =
        type;


    /*
     * length는 GUI에서 직접 입력받지 않고
     * payload 문자열 길이로 자동 설정한다.
     */
    state->response_header.length =
        (uint32_t)payload_length;


    /*
     * 요청과 응답을 매칭하기 위해
     * 요청 패킷의 seq를 자동으로 사용한다.
     */
    state->response_header.seq =
        state->request_header.seq;


    state->response_header.value =
        value;

    state->response_header.mode =
        mode;

    state->response_header.status =
        status;


    if (payload_length > 0U)
    {
        memcpy(
            state->response_payload,
            payload,
            payload_length
        );
    }


    state->response_ready = 1;


    /*
     * GUI 응답을 기다리던 통신 스레드를 깨운다.
     */
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