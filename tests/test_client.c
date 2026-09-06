#include "common/packet.h"
#include "common/tcp.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 5001

#define TEST_CLIENT_ID 100U
#define DEVICE_A_ID      1U


int main(void)
{
    int socket_fd;

    InternalMsgHeader_t
        host_header =
        {
            0
        };

    InternalMsgHeader_t
        network_header;

    const char payload[] =
        "HELLO DEVICE A";


    socket_fd =
        tcp_client_connect(
            SERVER_IP,
            SERVER_PORT
        );


    if (socket_fd < 0)
    {
        perror(
            "tcp_client_connect"
        );

        return 1;
    }


    host_header.msgType =
        PACKET_START;

    host_header.msgSize =
        (uint32_t)strlen(payload);


    if (internal_msg_header_set_realtime(
            &host_header) < 0)
    {
        perror(
            "internal_msg_header_set_realtime"
        );

        tcp_close(
            socket_fd
        );

        return 1;
    }


    host_header.srcId =
        TEST_CLIENT_ID;

    host_header.destId =
        DEVICE_A_ID;


    if (internal_msg_header_hton(
            &host_header,
            &network_header) < 0)
    {
        tcp_close(
            socket_fd
        );

        return 1;
    }


    if (tcp_send_all(
            socket_fd,
            &network_header,
            INTERNAL_MSG_HEADER_SIZE) < 0)
    {
        perror(
            "send header"
        );

        tcp_close(
            socket_fd
        );

        return 1;
    }


    if (host_header.msgSize > 0U)
    {
        if (tcp_send_all(
                socket_fd,
                payload,
                host_header.msgSize) < 0)
        {
            perror(
                "send payload"
            );

            tcp_close(
                socket_fd
            );

            return 1;
        }
    }


    printf(
        "Packet sent\n"
        "  msgType = %u\n"
        "  msgSize = %u\n"
        "  msgSec  = %u\n"
        "  msgNSec = %u\n"
        "  srcId   = %u\n"
        "  destId  = %u\n",
        (unsigned int)host_header.msgType,
        (unsigned int)host_header.msgSize,
        (unsigned int)host_header.msgSec,
        (unsigned int)host_header.msgNSec,
        (unsigned int)host_header.srcId,
        (unsigned int)host_header.destId
    );


    tcp_close(
        socket_fd
    );


    return 0;
}