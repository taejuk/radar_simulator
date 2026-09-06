#include "common/packet.h"
#include "common/tcp.h"
#include "config.h"
#include "device/device_a_packet.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


/*
 * Device A 클라이언트로 PACKET_START 요청을 보낸다.
 */
static int send_start_packet(
    int peer_fd,
    uint32_t seq)
{
    packet_header_t header;

    const char payload[] =
        "START DEVICE A";


    memset(
        &header,
        0,
        sizeof(header)
    );


    header.type =
        PACKET_START;

    header.length =
        (uint32_t)(sizeof(payload) - 1U);

    header.seq =
        seq;

    header.value =
        100U;

    header.mode =
        1U;

    header.status =
        0U;


    /*
     * Header 전송
     */
    if (tcp_send_all(
            peer_fd,
            &header,
            sizeof(header)) < 0)
    {
        perror("send request header");

        return -1;
    }


    /*
     * Payload 전송
     */
    if (tcp_send_all(
            peer_fd,
            payload,
            header.length) < 0)
    {
        perror("send request payload");

        return -1;
    }


    printf(
        "[Test Server] request sent\n"
        "  type    = %u\n"
        "  length  = %u\n"
        "  seq     = %u\n"
        "  value   = %u\n"
        "  mode    = %u\n"
        "  status  = %u\n"
        "  payload = \"%s\"\n",
        (unsigned int)header.type,
        (unsigned int)header.length,
        (unsigned int)header.seq,
        (unsigned int)header.value,
        (unsigned int)header.mode,
        (unsigned int)header.status,
        payload
    );


    return 0;
}

static int receive_response(
    int peer_fd)
{
    device_a_response_packet_t
        response_packet;

    ssize_t recv_size;


    recv_size = tcp_recv_exact(
        peer_fd,
        &response_packet,
        sizeof(response_packet)
    );


    if (recv_size == 0)
    {
        fprintf(
            stderr,
            "[Test Server] client disconnected "
            "before response\n"
        );

        return -1;
    }


    if (recv_size < 0)
    {
        perror(
            "receive response packet"
        );

        return -1;
    }


    printf(
        "[Test Server] response received\n"
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


    if (response_packet.message_type !=
        (uint16_t)PACKET_STATUS)
    {
        fprintf(
            stderr,
            "[Test Server] expected PACKET_STATUS\n"
        );

        return -1;
    }


    if (response_packet.time_nsec >
        999999999U)
    {
        fprintf(
            stderr,
            "[Test Server] invalid time_nsec\n"
        );

        return -1;
    }


    return 0;
}



int main(void)
{
    int listen_fd;
    int peer_fd;
    int exit_code = 1;

    const uint32_t seq = 1U;


    /*
     * Device A가 접속할 테스트 서버 생성
     */
    listen_fd = tcp_server_create(
        DEVICE_A_PORT
    );


    if (listen_fd < 0)
    {
        perror("tcp_server_create");

        return 1;
    }


    printf(
        "[Test Server] listening on port %u\n",
        DEVICE_A_PORT
    );


    /*
     * Device A 연결을 한 번만 받는다.
     */
    peer_fd = tcp_accept(
        listen_fd
    );


    if (peer_fd < 0)
    {
        perror("tcp_accept");

        tcp_close(
            listen_fd
        );

        return 1;
    }


    printf(
        "[Test Server] Device A connected\n"
    );


    /*
     * PACKET_START 요청을 한 번 전송한다.
     */
    if (send_start_packet(
            peer_fd,
            seq) < 0)
    {
        goto cleanup;
    }


    /*
     * Device A의 PACKET_STATUS 응답을 한 번 수신한다.
     */
    if (receive_response(
            peer_fd) < 0)
    {
        goto cleanup;
    }


    printf(
        "[Test Server] packet exchange passed\n"
    );


    exit_code = 0;


cleanup:

    tcp_close(
        peer_fd
    );

    tcp_close(
        listen_fd
    );


    printf(
        "[Test Server] test finished\n"
    );


    return exit_code;
}