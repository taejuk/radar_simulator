#include "device/device_handler.h"

#include "common/logger.h"
#include "common/packet.h"
#include "common/tcp.h"

#include "config.h"
#include "gui/device_gui_state.h"

#include <stdint.h>
#include <string.h>


int device_handler(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload)
{
    InternalMsgHeader_t
        response_header;

    InternalMsgHeader_t
        network_response_header;


    uint8_t response_payload[
        MAX_PAYLOAD_SIZE
    ];

    uint32_t response_payload_size =
        0U;

    uint16_t response_msg_type =
        0U;


    if ((ctx == NULL) ||
        (header == NULL) ||
        (ctx->gui_state == NULL))
    {
        return -1;
    }


    DEVICE_LOG_INFO(
        ctx->device_id,
        "PACKET_RECEIVED",
        "msgType=%u msgSize=%u "
        "srcId=%u destId=%u",
        (unsigned int)header->msgType,
        (unsigned int)header->msgSize,
        (unsigned int)header->srcId,
        (unsigned int)header->destId
    );


    /*
     * 해당 Device 탭에서 Send 버튼을 누를 때까지
     * 현재 통신 스레드만 기다린다.
     *
     * 다른 Device 통신 스레드는 영향을 받지 않는다.
     */
    if (device_gui_wait_for_response(
            ctx->gui_state,
            header,
            payload,
            &response_msg_type,
            response_payload,
            (uint32_t)sizeof(response_payload),
            &response_payload_size) < 0)
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "GUI_RESPONSE_WAIT_FAILED",
            "msgType=%u",
            (unsigned int)header->msgType
        );

        return -1;
    }


    memset(
        &response_header,
        0,
        sizeof(response_header)
    );


    response_header.msgType =
        response_msg_type;

    response_header.msgSize =
        response_payload_size;


    /*
     * 수신 Header의 출발지와 목적지를 바꾼다.
     */
    response_header.srcId =
        header->destId;

    response_header.destId =
        header->srcId;


    /*
     * Header의 시간은 GUI 입력값이 아니라
     * 실제 전송 시점의 CLOCK_REALTIME으로 설정한다.
     */
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
     * 응답 Header를 Big Endian으로 변환한다.
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
     * Header 전송
     */
    if (tcp_send_all(
            ctx->client_fd,
            &network_response_header,
            INTERNAL_MSG_HEADER_SIZE) < 0)
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "HEADER_SEND_FAILED",
            "msgType=%u",
            (unsigned int)response_header.msgType
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
     * Payload 전송
     *
     * 현재 단계에서는 기존 코드와 동일하게
     * payload 구조체를 byte buffer로 그대로 전송한다.
     */
    if (response_payload_size > 0U)
    {
        if (tcp_send_all(
                ctx->client_fd,
                response_payload,
                response_payload_size) < 0)
        {
            DEVICE_LOG_ERROR(
                ctx->device_id,
                "PAYLOAD_SEND_FAILED",
                "msgType=%u msgSize=%u",
                (unsigned int)response_header.msgType,
                (unsigned int)response_header.msgSize
            );

            return -1;
        }
    }


    DEVICE_LOG_INFO(
        ctx->device_id,
        "PACKET_SENT",
        "msgType=%u msgSize=%u",
        (unsigned int)response_header.msgType,
        (unsigned int)response_header.msgSize
    );


    return 0;
}