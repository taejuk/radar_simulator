#include "handler/device_a_handler.h"

#include "common/tcp.h"
#include "gui/device_a_gui_state.h"

#include <stdint.h>
#include <stdio.h>


int device_a_handle_start(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    packet_header_t response_header;

    uint8_t response_payload[
        MAX_PAYLOAD_SIZE
    ];


    if (ctx->gui_state == NULL)
    {
        fprintf(
            stderr,
            "[Device A] GUI state is NULL\n"
        );

        return -1;
    }


    printf(
        "[Device A] START packet received: seq=%u\n",
        (unsigned int)header->seq
    );


    /*
     * 1. 받은 요청을 GUI에 표시한다.
     * 2. 사용자가 Send 버튼을 누를 때까지 기다린다.
     * 3. GUI가 작성한 응답을 복사한다.
     */
    if (device_a_gui_wait_for_response(
            ctx->gui_state,
            header,
            payload,
            &response_header,
            response_payload) < 0)
    {
        fprintf(
            stderr,
            "[Device A] GUI response wait failed\n"
        );

        return -1;
    }


    /*
     * 실제 socket 전송은 통신 스레드가 담당한다.
     */
    if (tcp_send_all(
            ctx->client_fd,
            &response_header,
            sizeof(response_header)) < 0)
    {
        perror(
            "[Device A] response header send"
        );

        return -1;
    }


    if (response_header.length > 0U)
    {
        if (tcp_send_all(
                ctx->client_fd,
                response_payload,
                response_header.length) < 0)
        {
            perror(
                "[Device A] response payload send"
            );

            return -1;
        }
    }


    printf(
        "[Device A] GUI response sent: seq=%u\n",
        (unsigned int)response_header.seq
    );


    return 0;
}


int device_a_handle_status(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    (void)header;
    (void)payload;

    printf(
        "[Device A] STATUS packet received\n"
    );

    printf(
        "[Device A] state = %d\n",
        ctx->state
    );

    /*
     * STATUS 응답 시나리오
     */

    return 0;
}


int device_a_handle_control(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    (void)ctx;
    (void)payload;

    printf(
        "[Device A] CONTROL packet received\n"
    );

    printf(
        "[Device A] payload length = %u\n",
        (unsigned int)header->length
    );

    /*
     * CONTROL 시나리오
     */

    return 0;
}