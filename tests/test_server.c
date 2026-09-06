#include "common/packet.h"
#include "common/tcp.h"

#include "device/device_a_packet.h"

#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define TEST_SERVER_ID 100U
#define DEVICE_A_ID      1U


static int send_internal_header(
    int socket_fd,
    const InternalMsgHeader_t *host_header)
{
    InternalMsgHeader_t
        network_header;


    if (internal_msg_header_hton(
            host_header,
            &network_header) < 0)
    {
        return -1;
    }


    if (tcp_send_all(
            socket_fd,
            &network_header,
            INTERNAL_MSG_HEADER_SIZE) < 0)
    {
        return -1;
    }


    return 0;
}


static int receive_internal_header(
    int socket_fd,
    InternalMsgHeader_t *host_header)
{
    InternalMsgHeader_t
        network_header;

    ssize_t recv_size;


    recv_size = tcp_recv_exact(
        socket_fd,
        &network_header,
        INTERNAL_MSG_HEADER_SIZE
    );


    if (recv_size <= 0)
    {
        return -1;
    }


    if ((size_t)recv_size !=
        INTERNAL_MSG_HEADER_SIZE)
    {
        return -1;
    }


    if (internal_msg_header_ntoh(
            &network_header,
            host_header) < 0)
    {
        return -1;
    }


    if (internal_msg_header_validate(
            host_header) < 0)
    {
        return -1;
    }


    return 0;
}


static int send_start_packet(
    int peer_fd)
{
    InternalMsgHeader_t
        request_header =
        {
            0
        };

    const char request_payload[] =
        "START DEVICE A";


    request_header.msgType =
        PACKET_START;

    request_header.msgSize =
        (uint32_t)(
            sizeof(request_payload) - 1U
        );


    if (internal_msg_header_set_realtime(
            &request_header) < 0)
    {
        perror(
            "internal_msg_header_set_realtime"
        );

        return -1;
    }


    request_header.srcId =
        TEST_SERVER_ID;

    request_header.destId =
        DEVICE_A_ID;


    if (send_internal_header(
            peer_fd,
            &request_header) < 0)
    {
        perror(
            "send request header"
        );

        return -1;
    }


    if (tcp_send_all(
            peer_fd,
            request_payload,
            request_header.msgSize) < 0)
    {
        perror(
            "send request payload"
        );

        return -1;
    }


    printf(
        "[Test Server] request sent\n"
        "  msgType = %u\n"
        "  msgSize = %u\n"
        "  msgSec  = %u\n"
        "  msgNSec = %u\n"
        "  srcId   = %u\n"
        "  destId  = %u\n",
        (unsigned int)request_header.msgType,
        (unsigned int)request_header.msgSize,
        (unsigned int)request_header.msgSec,
        (unsigned int)request_header.msgNSec,
        (unsigned int)request_header.srcId,
        (unsigned int)request_header.destId
    );


    return 0;
}


static int receive_response(
    int peer_fd)
{
    InternalMsgHeader_t
        response_header;

    device_a_response_packet_t
        response_packet;

    ssize_t recv_size;


    /*
     * 응답 Header 먼저 수신
     */
    if (receive_internal_header(
            peer_fd,
            &response_header) < 0)
    {
        fprintf(
            stderr,
            "[Test Server] response header failed\n"
        );

        return -1;
    }


    printf(
        "[Test Server] response header received\n"
        "  msgType = %u\n"
        "  msgSize = %u\n"
        "  msgSec  = %u\n"
        "  msgNSec = %u\n"
        "  srcId   = %u\n"
        "  destId  = %u\n",
        (unsigned int)response_header.msgType,
        (unsigned int)response_header.msgSize,
        (unsigned int)response_header.msgSec,
        (unsigned int)response_header.msgNSec,
        (unsigned int)response_header.srcId,
        (unsigned int)response_header.destId
    );


    if (response_header.msgSize !=
        sizeof(response_packet))
    {
        fprintf(
            stderr,
            "[Test Server] unexpected payload size: "
            "received=%u expected=%u\n",
            (unsigned int)response_header.msgSize,
            (unsigned int)sizeof(response_packet)
        );

        return -1;
    }


    /*
     * Header에 기록된 크기만큼 payload를 수신한다.
     */
    recv_size = tcp_recv_exact(
        peer_fd,
        &response_packet,
        response_header.msgSize
    );


    if (recv_size <= 0)
    {
        fprintf(
            stderr,
            "[Test Server] response payload failed\n"
        );

        return -1;
    }


    printf(
        "[Test Server] response payload received\n"
        "  message_type = %u\n"
        "  time_sec     = %u\n"
        "  time_nsec    = %u\n"
        "  radar_status = %u\n"
        "  mode         = %u\n"
        "  status       = %u\n",
        (unsigned int)
            response_packet.message_type,
        (unsigned int)
            response_packet.time_sec,
        (unsigned int)
            response_packet.time_nsec,
        (unsigned int)
            response_packet.radar_status,
        (unsigned int)
            response_packet.mode,
        (unsigned int)
            response_packet.status
    );


    /*
     * srcId와 destId가 교환되었는지 확인한다.
     */
    if ((response_header.srcId !=
         DEVICE_A_ID) ||
        (response_header.destId !=
         TEST_SERVER_ID))
    {
        fprintf(
            stderr,
            "[Test Server] srcId/destId swap failed\n"
        );

        return -1;
    }


    return 0;
}


int main(void)
{
    int listen_fd;
    int peer_fd;

    int exit_code =
        1;


    listen_fd =
        tcp_server_create(
            DEVICE_A_PORT
        );


    if (listen_fd < 0)
    {
        perror(
            "tcp_server_create"
        );

        return 1;
    }


    printf(
        "[Test Server] listening on port %u\n",
        DEVICE_A_PORT
    );


    peer_fd =
        tcp_accept(
            listen_fd
        );


    if (peer_fd < 0)
    {
        perror(
            "tcp_accept"
        );

        tcp_close(
            listen_fd
        );

        return 1;
    }


    printf(
        "[Test Server] Device A connected\n"
    );


    if (send_start_packet(
            peer_fd) < 0)
    {
        goto cleanup;
    }


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