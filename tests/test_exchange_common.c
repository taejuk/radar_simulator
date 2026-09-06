#include "test_exchange_common.h"

#include "common/packet.h"
#include "common/tcp.h"
#include "config.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


#define TEST_PEER_ID 100U


static int send_request(
    int socket_fd,
    const char *device_name,
    uint8_t device_id)
{
    InternalMsgHeader_t
        host_header = {0};

    InternalMsgHeader_t
        network_header;

    char payload[64];

    int payload_length;


    payload_length = snprintf(
        payload,
        sizeof(payload),
        "START DEVICE %s",
        device_name
    );


    if ((payload_length < 0) ||
        ((size_t)payload_length >=
         sizeof(payload)))
    {
        return -1;
    }


    host_header.msgType =
        PACKET_START;

    host_header.msgSize =
        (uint32_t)payload_length;

    host_header.srcId =
        TEST_PEER_ID;

    host_header.destId =
        device_id;


    if (internal_msg_header_set_realtime(
            &host_header) < 0)
    {
        return -1;
    }


    if (internal_msg_header_hton(
            &host_header,
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


    if (host_header.msgSize > 0U)
    {
        if (tcp_send_all(
                socket_fd,
                payload,
                host_header.msgSize) < 0)
        {
            return -1;
        }
    }


    printf(
        "[Test %s] request sent: "
        "msgType=%u msgSize=%u "
        "srcId=%u destId=%u\n",
        device_name,
        (unsigned int)host_header.msgType,
        (unsigned int)host_header.msgSize,
        (unsigned int)host_header.srcId,
        (unsigned int)host_header.destId
    );


    return 0;
}


static int receive_response(
    int socket_fd,
    const char *device_name,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer)
{
    InternalMsgHeader_t
        network_header;

    InternalMsgHeader_t
        host_header;

    ssize_t recv_size;


    recv_size = tcp_recv_exact(
        socket_fd,
        &network_header,
        INTERNAL_MSG_HEADER_SIZE
    );


    if ((recv_size <= 0) ||
        ((size_t)recv_size !=
         INTERNAL_MSG_HEADER_SIZE))
    {
        fprintf(
            stderr,
            "[Test %s] response header failed\n",
            device_name
        );

        return -1;
    }


    if (internal_msg_header_ntoh(
            &network_header,
            &host_header) < 0)
    {
        fprintf(
            stderr,
            "[Test %s] header ntoh failed\n",
            device_name
        );

        return -1;
    }


    if (internal_msg_header_validate(
            &host_header) < 0)
    {
        fprintf(
            stderr,
            "[Test %s] invalid response header\n",
            device_name
        );

        return -1;
    }


    if (host_header.msgSize !=
        response_packet_size)
    {
        fprintf(
            stderr,
            "[Test %s] payload size mismatch: "
            "received=%u expected=%u\n",
            device_name,
            (unsigned int)host_header.msgSize,
            (unsigned int)response_packet_size
        );

        return -1;
    }


    if ((host_header.srcId != device_id) ||
        (host_header.destId != TEST_PEER_ID))
    {
        fprintf(
            stderr,
            "[Test %s] srcId/destId swap failed: "
            "srcId=%u destId=%u\n",
            device_name,
            (unsigned int)host_header.srcId,
            (unsigned int)host_header.destId
        );

        return -1;
    }


    recv_size = tcp_recv_exact(
        socket_fd,
        response_packet,
        response_packet_size
    );


    if ((recv_size <= 0) ||
        ((uint32_t)recv_size !=
         response_packet_size))
    {
        fprintf(
            stderr,
            "[Test %s] response payload failed\n",
            device_name
        );

        return -1;
    }


    printf(
        "[Test %s] response header received\n"
        "  msgType = %u\n"
        "  msgSize = %u\n"
        "  msgSec  = %u\n"
        "  msgNSec = %u\n"
        "  srcId   = %u\n"
        "  destId  = %u\n",
        device_name,
        (unsigned int)host_header.msgType,
        (unsigned int)host_header.msgSize,
        (unsigned int)host_header.msgSec,
        (unsigned int)host_header.msgNSec,
        (unsigned int)host_header.srcId,
        (unsigned int)host_header.destId
    );


    response_printer(
        response_packet
    );


    return 0;
}


static int run_exchange(
    int socket_fd,
    const char *device_name,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer)
{
    if (send_request(
            socket_fd,
            device_name,
            device_id) < 0)
    {
        fprintf(
            stderr,
            "[Test %s] request send failed\n",
            device_name
        );

        return -1;
    }


    if (receive_response(
            socket_fd,
            device_name,
            device_id,
            response_packet,
            response_packet_size,
            response_printer) < 0)
    {
        return -1;
    }


    printf(
        "[Test %s] packet exchange passed\n",
        device_name
    );


    return 0;
}


int test_run_client(
    const char *device_name,
    uint16_t device_port,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer)
{
    int socket_fd;

    int result;


    if ((device_name == NULL) ||
        (response_packet == NULL) ||
        (response_printer == NULL))
    {
        return 1;
    }


    socket_fd = tcp_client_connect(
        SERVER_IP,
        device_port
    );


    if (socket_fd < 0)
    {
        perror(
            "tcp_client_connect"
        );

        return 1;
    }


    printf(
        "[Test Client %s] connected to %s:%u\n",
        device_name,
        SERVER_IP,
        (unsigned int)device_port
    );


    result = run_exchange(
        socket_fd,
        device_name,
        device_id,
        response_packet,
        response_packet_size,
        response_printer
    );


    tcp_close(
        socket_fd
    );


    return (result == 0) ?
        0 : 1;
}


int test_run_server(
    const char *device_name,
    uint16_t device_port,
    uint8_t device_id,
    void *response_packet,
    uint32_t response_packet_size,
    test_response_printer_t response_printer)
{
    int listen_fd;

    int peer_fd;

    int result;


    if ((device_name == NULL) ||
        (response_packet == NULL) ||
        (response_printer == NULL))
    {
        return 1;
    }


    listen_fd = tcp_server_create(
        device_port
    );


    if (listen_fd < 0)
    {
        perror(
            "tcp_server_create"
        );

        return 1;
    }


    printf(
        "[Test Server %s] listening on port %u\n",
        device_name,
        (unsigned int)device_port
    );


    peer_fd = tcp_accept(
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
        "[Test Server %s] device connected\n",
        device_name
    );


    result = run_exchange(
        peer_fd,
        device_name,
        device_id,
        response_packet,
        response_packet_size,
        response_printer
    );


    tcp_close(
        peer_fd
    );

    tcp_close(
        listen_fd
    );


    return (result == 0) ?
        0 : 1;
}