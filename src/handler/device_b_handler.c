#include "handler/device_b_handler.h"

#include <stdio.h>


int device_b_handle_start(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    (void)payload;

    printf(
        "[Device B] START packet received\n"
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
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    (void)header;
    (void)payload;


    /*
     * STATUS 응답 시나리오
     */

    return 0;
}


int device_b_handle_control(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    (void)ctx;
    (void)payload;


    /*
     * CONTROL 시나리오
     */

    return 0;
}