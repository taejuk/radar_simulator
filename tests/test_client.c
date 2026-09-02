#include "common/packet.h"
#include "common/tcp.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 5001


int main(void)
{
    int socket_fd;

    struct sockaddr_in server_addr;

    packet_header_t header;

    const char payload[] =
        "HELLO DEVICE A";


    /*
     * =====================================
     * 1. Client socket 생성
     * =====================================
     */

    socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );


    if (socket_fd < 0)
    {
        perror("socket");

        return 1;
    }


    /*
     * =====================================
     * 2. Server 주소 설정
     * =====================================
     */

    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );


    server_addr.sin_family =
        AF_INET;


    server_addr.sin_port =
        htons(SERVER_PORT);


    if (inet_pton(
            AF_INET,
            SERVER_IP,
            &server_addr.sin_addr) != 1)
    {
        perror("inet_pton");

        close(socket_fd);

        return 1;
    }


    /*
     * =====================================
     * 3. Server 연결
     * =====================================
     */

    printf(
        "Connecting to %s:%d...\n",
        SERVER_IP,
        SERVER_PORT
    );


    if (connect(
            socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("connect");

        close(socket_fd);

        return 1;
    }


    printf("Connected\n");


    /*
     * =====================================
     * 4. Header 구성
     * =====================================
     *
     * 현재 서버 코드에서
     * PACKET_START = 1
     */

    memset(
        &header,
        0,
        sizeof(header)
    );


    header.type =
        PACKET_START;


    /*
     * payload 끝의 '\0'은 전송하지 않음
     */
    header.length =
        (uint32_t)strlen(payload);


    header.seq =
        1;


    header.value =
        100;


    header.mode =
        1;


    header.status =
        0;


    printf(
        "Header size = %zu\n",
        sizeof(header)
    );


    printf(
        "Send Header\n"
        "  type   = %u\n"
        "  length = %u\n"
        "  seq    = %u\n"
        "  value  = %u\n"
        "  mode   = %u\n"
        "  status = %u\n",
        (unsigned int)header.type,
        (unsigned int)header.length,
        (unsigned int)header.seq,
        (unsigned int)header.value,
        (unsigned int)header.mode,
        (unsigned int)header.status
    );


    /*
     * =====================================
     * 5. Header 전송
     * =====================================
     */

    if (tcp_send_all(
            socket_fd,
            &header,
            sizeof(header)) < 0)
    {
        perror("send header");

        close(socket_fd);

        return 1;
    }


    /*
     * =====================================
     * 6. Payload 전송
     * =====================================
     */

    if (header.length > 0)
    {
        if (tcp_send_all(
                socket_fd,
                payload,
                header.length) < 0)
        {
            perror("send payload");

            close(socket_fd);

            return 1;
        }
    }


    printf(
        "Payload sent: \"%s\"\n",
        payload
    );


    printf(
        "Packet send complete\n"
    );


    /*
     * 테스트에서는 전송 완료 후 종료
     */
    close(socket_fd);


    return 0;
}