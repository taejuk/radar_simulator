#include "common/comm_thread.h"

#include "common/packet.h"
#include "common/tcp.h"
#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>


#define CONNECT_RETRY_SECONDS 1U


/*
 * is_server 값에 따라 accept 또는 connect를 수행한다.
 */
static int comm_open_connection(
    device_context_t *ctx)
{
    if (ctx->is_server == 1)
    {
        return tcp_accept(
            ctx->listen_fd
        );
    }


    printf(
        "[Device %d] connecting to %s:%u\n",
        ctx->device_id,
        SERVER_IP,
        ctx->port
    );


    return tcp_client_connect(
        SERVER_IP,
        ctx->port
    );
}


/*
 * 연결된 socket으로부터 패킷을 계속 수신한다.
 */
static void comm_receive_packets(
    device_context_t *ctx)
{
    while (1)
    {
        packet_header_t header;

        uint8_t payload[MAX_PAYLOAD_SIZE];

        ssize_t recv_size;


        /*
         * Header 수신
         */
        recv_size = tcp_recv_exact(
            ctx->client_fd,
            &header,
            sizeof(header)
        );


        if (recv_size == 0)
        {
            /*
             * 상대방이 정상적으로 연결 종료
             */
            break;
        }


        if (recv_size < 0)
        {
            fprintf(
                stderr,
                "[Device %d] header recv failed\n",
                ctx->device_id
            );

            break;
        }


        /*
         * Header 검증
         */
        if (packet_header_validate(
                &header) < 0)
        {
            fprintf(
                stderr,
                "[Device %d] invalid header\n",
                ctx->device_id
            );

            break;
        }


        printf(
            "[Device %d] "
            "type=%u length=%u seq=%u\n",
            ctx->device_id,
            (unsigned int)header.type,
            (unsigned int)header.length,
            (unsigned int)header.seq
        );


        /*
         * Payload 수신
         */
        if (header.length > 0U)
        {
            recv_size = tcp_recv_exact(
                ctx->client_fd,
                payload,
                header.length
            );


            if (recv_size == 0)
            {
                break;
            }


            if (recv_size < 0)
            {
                fprintf(
                    stderr,
                    "[Device %d] payload recv failed\n",
                    ctx->device_id
                );

                break;
            }
        }


        /*
         * 장비별 handler 호출
         */
        if (ctx->packet_handler != NULL)
        {
            int ret;

            ret = ctx->packet_handler(
                ctx,
                &header,
                payload
            );


            if (ret < 0)
            {
                fprintf(
                    stderr,
                    "[Device %d] packet handler failed\n",
                    ctx->device_id
                );
            }
        }
    }
}


void *comm_thread(
    void *arg)
{
    device_context_t *ctx =
        (device_context_t *)arg;


    /*
     * is_server는 0 또는 1만 허용한다.
     */
    if ((ctx->is_server != 0) &&
        (ctx->is_server != 1))
    {
        fprintf(
            stderr,
            "[Device %d] is_server must be 0 or 1\n",
            ctx->device_id
        );

        return NULL;
    }


    /*
     * =================================
     * Server 모드 초기화
     * =================================
     */
    if (ctx->is_server == 1)
    {
        ctx->listen_fd =
            tcp_server_create(ctx->port);


        if (ctx->listen_fd < 0)
        {
            fprintf(
                stderr,
                "[Device %d] server create failed\n",
                ctx->device_id
            );

            return NULL;
        }


        printf(
            "[Device %d] listening on port %u\n",
            ctx->device_id,
            ctx->port
        );
    }


    /*
     * =================================
     * Connection Loop
     * =================================
     *
     * server: accept 반복
     * client: connect 반복
     */
    while (1)
    {
        ctx->client_fd =
            comm_open_connection(ctx);


        if (ctx->client_fd < 0)
        {
            fprintf(
                stderr,
                "[Device %d] %s failed\n",
                ctx->device_id,
                (ctx->is_server == 1) ?
                    "accept" : "connect"
            );


            /*
             * Client 모드에서 연결에 실패하면
             * CPU를 계속 사용하지 않도록 잠시 대기한다.
             */
            if (ctx->is_server == 0)
            {
                sleep(
                    CONNECT_RETRY_SECONDS
                );
            }


            continue;
        }


        printf(
            "[Device %d] peer connected (%s mode)\n",
            ctx->device_id,
            (ctx->is_server == 1) ?
                "server" : "client"
        );


        /*
         * Header + Payload 수신 및 handler 호출
         */
        comm_receive_packets(ctx);


        printf(
            "[Device %d] peer disconnected\n",
            ctx->device_id
        );


        tcp_close(
            ctx->client_fd
        );

        ctx->client_fd = -1;


        /*
         * server이면 다시 accept()
         * client이면 다시 connect()
         */
    }


    return NULL;
}