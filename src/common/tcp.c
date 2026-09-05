#include "common/tcp.h"
#include "config.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

int tcp_server_create(
    uint16_t port)
{
    int socket_fd;

    int reuse = 1;

    struct sockaddr_in address;


    socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        return -1;
    }


    if (setsockopt(
            socket_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)) < 0)
    {
        close(socket_fd);

        return -1;
    }


    address.sin_family = AF_INET;

    address.sin_addr.s_addr =
        htonl(INADDR_ANY);

    address.sin_port =
        htons(port);


    if (bind(
            socket_fd,
            (struct sockaddr *)&address,
            sizeof(address)) < 0)
    {
        close(socket_fd);

        return -1;
    }


    /*
     * 현재 Thread 하나가 연결 하나를
     * 전담하므로 backlog = 1로 설정.
     */
    if (listen(
            socket_fd,
            LISTEN_BACKLOG) < 0)
    {
        close(socket_fd);

        return -1;
    }


    return socket_fd;
}

int tcp_client_connect(
    const char *server_ip,
    uint16_t port)
{
    int socket_fd;

    struct sockaddr_in server_address;


    if (server_ip == NULL)
    {
        errno = EINVAL;

        return -1;
    }


    /*
     * TCP socket 생성
     */
    socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (socket_fd < 0)
    {
        return -1;
    }


    /*
     * 연결할 서버 주소 설정
     */
    memset(
        &server_address,
        0,
        sizeof(server_address)
    );

    server_address.sin_family = AF_INET;

    server_address.sin_port =
        htons(port);


    /*
     * 문자열 IP 주소를 네트워크 주소로 변환
     */
    if (inet_pton(
            AF_INET,
            server_ip,
            &server_address.sin_addr) != 1)
    {
        close(socket_fd);

        errno = EINVAL;

        return -1;
    }


    /*
     * 서버에 연결
     */
    if (connect(
            socket_fd,
            (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0)
    {
        close(socket_fd);

        return -1;
    }


    return socket_fd;
}


int tcp_accept(
    int listen_fd)
{
    while (1)
    {
        int client_fd;


        client_fd = accept(
            listen_fd,
            NULL,
            NULL
        );


        if (client_fd >= 0)
        {
            return client_fd;
        }


        if (errno == EINTR)
        {
            continue;
        }


        return -1;
    }
}



ssize_t tcp_recv_exact(
    int socket_fd,
    void *buffer,
    size_t length)
{
    size_t received = 0U;

    uint8_t *current =
        (uint8_t *)buffer;


    while (received < length)
    {
        ssize_t ret;


        ret = recv(
            socket_fd,
            current + received,
            length - received,
            0
        );


        /*
         * Peer가 정상 종료
         */
        if (ret == 0)
        {
            return 0;
        }


        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }


            return -1;
        }


        received += (size_t)ret;
    }


    return (ssize_t)received;
}



ssize_t tcp_send_all(
    int socket_fd,
    const void *buffer,
    size_t length)
{
    size_t sent = 0U;

    const uint8_t *current =
        (const uint8_t *)buffer;


    while (sent < length)
    {
        ssize_t ret;


        ret = send(
            socket_fd,
            current + sent,
            length - sent,
            MSG_NOSIGNAL
        );


        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }


            return -1;
        }


        sent += (size_t)ret;
    }


    return (ssize_t)sent;
}



int tcp_close(
    int socket_fd)
{
    if (socket_fd < 0)
    {
        return 0;
    }


    return close(socket_fd);
}