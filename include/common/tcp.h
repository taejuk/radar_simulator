#ifndef TCP_H
#define TCP_H


#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>


int tcp_server_create(
    uint16_t port
);


int tcp_accept(
    int listen_fd
);


ssize_t tcp_recv_exact(
    int socket_fd,
    void *buffer,
    size_t length
);


ssize_t tcp_send_all(
    int socket_fd,
    const void *buffer,
    size_t length
);


int tcp_close(
    int socket_fd
);


#endif