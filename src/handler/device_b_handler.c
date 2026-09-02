#include "handler/device_b_handler.h"

#include <stdio.h>


int device_b_handle_start(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    (void)payload;

    printf(
        "[Device A] START packet received\n"
    );

    printf(
        "[Device A] seq = %u\n",
        (unsigned int)header->seq
    );

    /*
     * 실제 START 시나리오
     *
     * 1. Payload 확인
     * 2. 내부 상태 변경
     * 3. 응답 Packet 생성
     * 4. send
     */

    ctx->state = 1;

    return 0;
}


int device_b_handle_status(
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


int device_b_handle_control(
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