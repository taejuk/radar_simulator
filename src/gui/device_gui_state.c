#include "gui/device_gui_state.h"

#include <stddef.h>
#include <string.h>


int device_gui_state_init(
    device_gui_state_t *state)
{
    int ret;


    if (state == NULL)
    {
        return -1;
    }


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


void device_gui_state_destroy(
    device_gui_state_t *state)
{
    if (state == NULL)
    {
        return;
    }


    pthread_cond_destroy(
        &state->response_condition
    );

    pthread_mutex_destroy(
        &state->mutex
    );
}


int device_gui_wait_for_response(
    device_gui_state_t *state,
    const InternalMsgHeader_t *request_header,
    const uint8_t *request_payload,
    uint16_t *response_msg_type,
    uint8_t *response_payload,
    uint32_t response_capacity,
    uint32_t *response_payload_size)
{
    int ret;


    if ((state == NULL) ||
        (request_header == NULL) ||
        (response_msg_type == NULL) ||
        (response_payload == NULL) ||
        (response_payload_size == NULL))
    {
        return -1;
    }


    if (request_header->msgSize >
        MAX_PAYLOAD_SIZE)
    {
        return -1;
    }


    if ((request_header->msgSize > 0U) &&
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


    state->request_header =
        *request_header;


    if (request_header->msgSize > 0U)
    {
        memcpy(
            state->request_payload,
            request_payload,
            request_header->msgSize
        );
    }


    /*
     * 문자열 테스트 payload 출력도 가능하도록
     * 마지막에 NULL 문자를 추가한다.
     */
    state->request_payload[
        request_header->msgSize
    ] = '\0';


    state->request_generation +=
        1U;

    state->request_pending = 1;

    state->response_ready = 0;


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


    if (state->response_payload_size >
        response_capacity)
    {
        state->request_pending = 0;

        state->response_ready = 0;

        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    *response_msg_type =
        state->response_msg_type;

    *response_payload_size =
        state->response_payload_size;


    if (state->response_payload_size > 0U)
    {
        memcpy(
            response_payload,
            state->response_payload,
            state->response_payload_size
        );
    }


    state->request_pending = 0;

    state->response_ready = 0;


    pthread_mutex_unlock(
        &state->mutex
    );


    return 0;
}


int device_gui_get_request(
    device_gui_state_t *state,
    InternalMsgHeader_t *request_header,
    uint8_t *request_payload,
    uint64_t *request_generation)
{
    int ret;

    int has_request;


    if ((state == NULL) ||
        (request_header == NULL) ||
        (request_payload == NULL) ||
        (request_generation == NULL))
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

    *request_generation =
        state->request_generation;


    if (has_request != 0)
    {
        *request_header =
            state->request_header;


        memcpy(
            request_payload,
            state->request_payload,
            state->request_header.msgSize + 1U
        );
    }
    else
    {
        memset(
            request_header,
            0,
            sizeof(*request_header)
        );

        request_payload[0] =
            '\0';
    }


    pthread_mutex_unlock(
        &state->mutex
    );


    return has_request;
}


int device_gui_submit_response(
    device_gui_state_t *state,
    uint16_t response_msg_type,
    const void *response_payload,
    uint32_t response_payload_size)
{
    int ret;


    if (state == NULL)
    {
        return -1;
    }


    if (response_payload_size >
        MAX_PAYLOAD_SIZE)
    {
        return -1;
    }


    if ((response_payload_size > 0U) &&
        (response_payload == NULL))
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


    if ((state->request_pending == 0) ||
        (state->response_ready != 0) ||
        (state->shutdown_requested != 0))
    {
        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


    state->response_msg_type =
        response_msg_type;

    state->response_payload_size =
        response_payload_size;


    if (response_payload_size > 0U)
    {
        memcpy(
            state->response_payload,
            response_payload,
            response_payload_size
        );
    }


    state->response_ready = 1;


    pthread_cond_signal(
        &state->response_condition
    );


    pthread_mutex_unlock(
        &state->mutex
    );


    return 0;
}


void device_gui_request_shutdown(
    device_gui_state_t *state)
{
    if (state == NULL)
    {
        return;
    }


    if (pthread_mutex_lock(
            &state->mutex) != 0)
    {
        return;
    }


    state->shutdown_requested = 1;


    pthread_cond_broadcast(
        &state->response_condition
    );


    pthread_mutex_unlock(
        &state->mutex
    );
}