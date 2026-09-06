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


    pthread_cond_destroy(
        &state->response_condition
    );

    pthread_mutex_destroy(
        &state->mutex
    );
}


int device_a_gui_wait_for_response(
    device_a_gui_state_t *state,
    const InternalMsgHeader_t *request_header,
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


    state->request_payload[
        request_header->msgSize
    ] = '\0';


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
    InternalMsgHeader_t *request_header,
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


    if ((state->request_pending == 0) ||
        (state->response_ready != 0) ||
        (state->shutdown_requested != 0))
    {
        pthread_mutex_unlock(
            &state->mutex
        );

        return -1;
    }


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


    pthread_cond_broadcast(
        &state->response_condition
    );


    pthread_mutex_unlock(
        &state->mutex
    );
}