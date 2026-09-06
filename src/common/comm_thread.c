#include "common/comm_thread.h"

#include "common/logger.h"
#include "common/packet.h"
#include "common/tcp.h"

#include "config.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>


static int comm_open_connection(
    device_context_t *ctx)
{
    if (ctx->is_server == 1)
    {
        DEVICE_LOG_INFO(
            ctx->device_id,
            "WAIT_CONNECTION",
            "port=%u",
            (unsigned int)ctx->port
        );

        return tcp_accept(
            ctx->listen_fd
        );
    }


    DEVICE_LOG_INFO(
        ctx->device_id,
        "CONNECTING",
        "server=%s port=%u",
        SERVER_IP,
        (unsigned int)ctx->port
    );


    return tcp_client_connect(
        SERVER_IP,
        ctx->port
    );
}


static void comm_receive_packets(
    device_context_t *ctx)
{
    while (1)
    {
        /*
         * socket에서 직접 받은 Big Endian Header
         */
        InternalMsgHeader_t
            network_header;

        /*
         * 프로그램 내부에서 사용할 Host Endian Header
         */
        InternalMsgHeader_t
            host_header;

        uint8_t payload[
            MAX_PAYLOAD_SIZE
        ];

        ssize_t recv_size;


        payload[0] = 0U;


        /*
         * =====================================
         * 16-byte Header 수신
         * =====================================
         */
        recv_size = tcp_recv_exact(
            ctx->client_fd,
            &network_header,
            INTERNAL_MSG_HEADER_SIZE
        );


        if (recv_size == 0)
        {
            DEVICE_LOG_INFO(
                ctx->device_id,
                "PEER_CLOSED",
                "phase=header"
            );

            break;
        }


        if (recv_size < 0)
        {
            const int saved_errno =
                errno;

            DEVICE_LOG_ERROR(
                ctx->device_id,
                "HEADER_RECV_FAILED",
                "errno=%d message=%s",
                saved_errno,
                strerror(saved_errno)
            );

            break;
        }


        if ((size_t)recv_size !=
            INTERNAL_MSG_HEADER_SIZE)
        {
            DEVICE_LOG_ERROR(
                ctx->device_id,
                "HEADER_SIZE_MISMATCH",
                "received=%u expected=%u",
                (unsigned int)recv_size,
                (unsigned int)
                    INTERNAL_MSG_HEADER_SIZE
            );

            break;
        }


        /*
         * Big Endian에서 Host Endian으로 변환한다.
         */
        if (internal_msg_header_ntoh(
                &network_header,
                &host_header) < 0)
        {
            DEVICE_LOG_ERROR(
                ctx->device_id,
                "HEADER_NTOH_FAILED",
                "conversion_failed=1"
            );

            break;
        }


        /*
         * Host Endian 상태에서 검증한다.
         */
        if (internal_msg_header_validate(
                &host_header) < 0)
        {
            DEVICE_LOG_WARNING(
                ctx->device_id,
                "INVALID_HEADER",
                "msgType=%u msgSize=%u "
                "msgSec=%u msgNSec=%u "
                "srcId=%u destId=%u",
                (unsigned int)host_header.msgType,
                (unsigned int)host_header.msgSize,
                (unsigned int)host_header.msgSec,
                (unsigned int)host_header.msgNSec,
                (unsigned int)host_header.srcId,
                (unsigned int)host_header.destId
            );

            break;
        }


        sim_log_packet(
            SIM_LOG_INFO,
            ctx->device_id,
            "RX_HEADER",
            &host_header
        );


        /*
         * =====================================
         * Payload 수신
         * =====================================
         */
        if (host_header.msgSize > 0U)
        {
            recv_size = tcp_recv_exact(
                ctx->client_fd,
                payload,
                host_header.msgSize
            );


            if (recv_size == 0)
            {
                DEVICE_LOG_INFO(
                    ctx->device_id,
                    "PEER_CLOSED",
                    "phase=payload msgType=%u",
                    (unsigned int)
                        host_header.msgType
                );

                break;
            }


            if (recv_size < 0)
            {
                const int saved_errno =
                    errno;

                DEVICE_LOG_ERROR(
                    ctx->device_id,
                    "PAYLOAD_RECV_FAILED",
                    "msgType=%u msgSize=%u "
                    "errno=%d message=%s",
                    (unsigned int)
                        host_header.msgType,
                    (unsigned int)
                        host_header.msgSize,
                    saved_errno,
                    strerror(saved_errno)
                );

                break;
            }


            if ((uint32_t)recv_size !=
                host_header.msgSize)
            {
                DEVICE_LOG_ERROR(
                    ctx->device_id,
                    "PAYLOAD_SIZE_MISMATCH",
                    "received=%u expected=%u",
                    (unsigned int)recv_size,
                    (unsigned int)
                        host_header.msgSize
                );

                break;
            }
        }


        /*
         * Handler에는 Host Endian Header를 전달한다.
         */
        if (ctx->packet_handler != NULL)
        {
            const int handler_result =
                ctx->packet_handler(
                    ctx,
                    &host_header,
                    payload
                );


            if (handler_result < 0)
            {
                DEVICE_LOG_ERROR(
                    ctx->device_id,
                    "HANDLER_FAILED",
                    "msgType=%u",
                    (unsigned int)
                        host_header.msgType
                );
            }
        }
        else
        {
            DEVICE_LOG_WARNING(
                ctx->device_id,
                "HANDLER_NOT_REGISTERED",
                "msgType=%u",
                (unsigned int)
                    host_header.msgType
            );
        }
    }
}


void *comm_thread(
    void *arg)
{
    device_context_t *ctx;


    if (arg == NULL)
    {
        sim_log_write(
            SIM_LOG_ERROR,
            -1,
            "INVALID_THREAD_ARGUMENT",
            "comm_thread argument is NULL"
        );

        return NULL;
    }


    ctx =
        (device_context_t *)arg;


    if ((ctx->is_server != 0) &&
        (ctx->is_server != 1))
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "INVALID_MODE",
            "is_server=%d",
            ctx->is_server
        );

        return NULL;
    }


    if (ctx->is_server == 1)
    {
        ctx->listen_fd =
            tcp_server_create(
                ctx->port
            );


        if (ctx->listen_fd < 0)
        {
            const int saved_errno =
                errno;

            DEVICE_LOG_ERROR(
                ctx->device_id,
                "SERVER_CREATE_FAILED",
                "port=%u errno=%d message=%s",
                (unsigned int)ctx->port,
                saved_errno,
                strerror(saved_errno)
            );

            return NULL;
        }


        DEVICE_LOG_INFO(
            ctx->device_id,
            "SERVER_LISTENING",
            "port=%u",
            (unsigned int)ctx->port
        );
    }


    while (1)
    {
        ctx->client_fd =
            comm_open_connection(
                ctx
            );


        if (ctx->client_fd < 0)
        {
            const int saved_errno =
                errno;

            DEVICE_LOG_ERROR(
                ctx->device_id,
                (ctx->is_server == 1) ?
                    "ACCEPT_FAILED" :
                    "CONNECT_FAILED",
                "port=%u errno=%d message=%s",
                (unsigned int)ctx->port,
                saved_errno,
                strerror(saved_errno)
            );


            /*
             * Client는 연결을 한 번만 시도한다.
             */
            if (ctx->is_server == 0)
            {
                return NULL;
            }


            continue;
        }


        DEVICE_LOG_INFO(
            ctx->device_id,
            "PEER_CONNECTED",
            "mode=%s socket=%d",
            (ctx->is_server == 1) ?
                "server" : "client",
            ctx->client_fd
        );


        comm_receive_packets(
            ctx
        );


        DEVICE_LOG_INFO(
            ctx->device_id,
            "PEER_DISCONNECTED",
            "socket=%d",
            ctx->client_fd
        );


        tcp_close(
            ctx->client_fd
        );

        ctx->client_fd = -1;


        if (ctx->is_server == 0)
        {
            DEVICE_LOG_INFO(
                ctx->device_id,
                "THREAD_FINISHED",
                "reason=communication_finished"
            );

            return NULL;
        }
    }


    return NULL;
}