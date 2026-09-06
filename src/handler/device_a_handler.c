#include "handler/device_a_handler.h"

#include "common/logger.h"
#include "common/packet.h"
#include "common/tcp.h"

#include "device/device_a_packet.h"
#include "gui/device_a_gui_state.h"

#include <stdint.h>
#include <stdio.h>


int device_a_handle_start(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    /*
     * GUI에서 입력한 payload
     */
    device_a_response_packet_t
        response_packet;

    /*
     * Host Endian 응답 Header
     */
    InternalMsgHeader_t
        response_header;

    /*
     * Big Endian 응답 Header
     */
    InternalMsgHeader_t
        network_response_header;


    if ((ctx == NULL) ||
        (header == NULL) ||
        (ctx->gui_state == NULL))
    {
        return -1;
    }


    DEVICE_LOG_INFO(
        ctx->device_id,
        "START_RECEIVED",
        "msgType=%u msgSize=%u "
        "srcId=%u destId=%u",
        (unsigned int)header->msgType,
        (unsigned int)header->msgSize,
        (unsigned int)header->srcId,
        (unsigned int)header->destId
    );


    /*
     * 사용자가 GUI에서 Send를 누를 때까지 기다린다.
     */
    if (device_a_gui_wait_for_response(
            ctx->gui_state,
            header,
            payload,
            &response_packet) < 0)
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "GUI_RESPONSE_WAIT_FAILED",
            "msgType=%u",
            (unsigned int)header->msgType
        );

        return -1;
    }


    /*
     * =====================================
     * 응답 Header 작성
     * =====================================
     */
    response_header.msgType =
        response_packet.message_type;

    /*
     * Header 뒤에 전송할 payload 크기
     */
    response_header.msgSize =
        (uint32_t)sizeof(response_packet);


    if (internal_msg_header_set_realtime(
            &response_header) < 0)
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "REALTIME_FAILED",
            "clock_gettime_failed=1"
        );

        return -1;
    }


    /*
     * 받은 Header의 출발지와 목적지를 서로 바꾼다.
     */
    response_header.srcId =
        header->destId;

    response_header.destId =
        header->srcId;


    /*
     * Host Endian Header를 Big Endian으로 변환한다.
     */
    if (internal_msg_header_hton(
            &response_header,
            &network_response_header) < 0)
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "HEADER_HTON_FAILED",
            "conversion_failed=1"
        );

        return -1;
    }


    /*
     * =====================================
     * Header 먼저 전송
     * =====================================
     */
    if (tcp_send_all(
            ctx->client_fd,
            &network_response_header,
            INTERNAL_MSG_HEADER_SIZE) < 0)
    {
        perror(
            "[Device A] response header send"
        );

        return -1;
    }


    sim_log_packet(
        SIM_LOG_INFO,
        ctx->device_id,
        "TX_HEADER",
        &response_header
    );


    /*
     * =====================================
     * Payload 전송
     *
     * 이번 단계에서는 payload 직렬화는
     * 변경하지 않고 구조체를 그대로 보낸다.
     * =====================================
     */
    if (response_header.msgSize > 0U)
    {
        if (tcp_send_all(
                ctx->client_fd,
                &response_packet,
                response_header.msgSize) < 0)
        {
            perror(
                "[Device A] response payload send"
            );

            return -1;
        }
    }


    DEVICE_LOG_INFO(
        ctx->device_id,
        "TX_PAYLOAD",
        "msgType=%u msgSize=%u",
        (unsigned int)response_header.msgType,
        (unsigned int)response_header.msgSize
    );


    return 0;
}


int device_a_handle_status(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    (void)payload;


    DEVICE_LOG_INFO(
        ctx->device_id,
        "STATUS_RECEIVED",
        "msgType=%u msgSize=%u",
        (unsigned int)header->msgType,
        (unsigned int)header->msgSize
    );


    return 0;
}


int device_a_handle_control(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    (void)payload;


    DEVICE_LOG_INFO(
        ctx->device_id,
        "CONTROL_RECEIVED",
        "msgType=%u msgSize=%u",
        (unsigned int)header->msgType,
        (unsigned int)header->msgSize
    );


    return 0;
}