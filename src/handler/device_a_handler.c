#include "handler/device_a_handler.h"

#include "common/tcp.h"
#include "gui/device_a_gui_state.h"
#include "device/device_a_packet.h"

#include <stdint.h>
#include <stdio.h>

int device_a_handle_start(
    device_context_t *ctx,
    const packet_header_t *header,
    const uint8_t *payload)
{
    device_a_response_packet_t
        response_packet;


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
     * 받은 요청을 GUI에 표시하고
     * 사용자가 Send를 누를 때까지 기다린다.
     */
    if (device_a_gui_wait_for_response(
            ctx->gui_state,
            header,
            payload,
            &response_packet) < 0)
    {
        fprintf(
            stderr,
            "[Device A] GUI response wait failed\n"
        );

        return -1;
    }


    /*
     * GUI에서 입력한 응답 구조체를 전송한다.
     */
    if (tcp_send_all(
            ctx->client_fd,
            &response_packet,
            sizeof(response_packet)) < 0)
    {
        perror(
            "[Device A] response packet send"
        );

        return -1;
    }


    printf(
        "[Device A] response sent\n"
        "  message_type = %u\n"
        "  time_sec     = %u\n"
        "  time_nsec    = %u\n"
        "  radar_status = %u\n"
        "  mode         = %u\n"
        "  status       = %u\n",
        (unsigned int)response_packet.message_type,
        (unsigned int)response_packet.time_sec,
        (unsigned int)response_packet.time_nsec,
        (unsigned int)response_packet.radar_status,
        (unsigned int)response_packet.mode,
        (unsigned int)response_packet.status
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