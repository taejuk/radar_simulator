#include "handler/device_a_handler.h"
#include "common/tcp.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int device_a_handle_start(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    packet_header_t response_header;

    const char response_payload[] =
        "DEVICE A STARTED";


    (void)payload;


    printf(
        "[Device A] START packet received\n"
    );


    printf(
        "[Device A] seq = %u\n",
        (unsigned int)header->seq
    );


    /*
     * Device A 상태 변경
     */
    ctx->state = 1;


    /*
     * 응답 Header 초기화
     */
    memset(
        &response_header,
        0,
        sizeof(response_header)
    );


    /*
     * 나중에는 이 부분의 값을
     * ImGui에서 설정한 값으로 가져오면 된다.
     */
    response_header.type =
        PACKET_STATUS;

    response_header.length =
        (uint32_t)(
            sizeof(response_payload) - 1U
        );

    /*
     * 요청과 응답을 매칭하기 위해
     * 요청 패킷의 seq를 그대로 사용한다.
     */
    response_header.seq =
        header->seq;

    response_header.value =
        (uint32_t)ctx->state;

    response_header.mode =
        header->mode;

    response_header.status =
        0U;


    /*
     * 응답 Header 전송
     */
    if (tcp_send_all(
            ctx->client_fd,
            &response_header,
            sizeof(response_header)) < 0)
    {
        perror(
            "[Device A] send response header"
        );

        return -1;
    }


    /*
     * 응답 Payload 전송
     */
    if (tcp_send_all(
            ctx->client_fd,
            response_payload,
            response_header.length) < 0)
    {
        perror(
            "[Device A] send response payload"
        );

        return -1;
    }


    printf(
        "[Device A] STATUS response sent\n"
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