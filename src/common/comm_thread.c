#include "common/comm_thread.h"

#include "common/logger.h"
#include "common/packet.h"
#include "common/tcp.h"

#include "config.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/*
 * is_server 값에 따라
 * accept 또는 connect를 수행한다.
 */
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


/*
 * 연결된 socket으로부터 패킷을 계속 수신한다.
 */
static void comm_receive_packets(
    device_context_t *ctx)
{
    while (1)
    {
        packet_header_t header =
        {
            0
        };

        uint8_t payload[
            MAX_PAYLOAD_SIZE
        ];

        ssize_t recv_size;


        /*
         * payload가 없는 패킷이 Handler로 전달되더라도
         * 첫 번째 값이 정의되어 있도록 초기화한다.
         */
        payload[0] = 0U;


        /*
         * =====================================
         * Header 수신
         * =====================================
         */
        recv_size = tcp_recv_exact(
            ctx->client_fd,
            &header,
            sizeof(header)
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
                "socket=%d errno=%d message=%s",
                ctx->client_fd,
                saved_errno,
                strerror(saved_errno)
            );

            break;
        }


        /*
         * Header 전체 크기를 받았는지 검사한다.
         *
         * tcp_recv_exact()이 정상적으로 구현됐다면
         * 성공 시 항상 sizeof(header)를 반환해야 한다.
         */
        if ((size_t)recv_size !=
            sizeof(header))
        {
            DEVICE_LOG_ERROR(
                ctx->device_id,
                "HEADER_SIZE_MISMATCH",
                "received=%u expected=%u",
                (unsigned int)recv_size,
                (unsigned int)sizeof(header)
            );

            break;
        }


        /*
         * =====================================
         * Header 검증
         * =====================================
         */
        if (packet_header_validate(
                &header) < 0)
        {
            DEVICE_LOG_WARNING(
                ctx->device_id,
                "INVALID_HEADER",
                "type=%u length=%u seq=%u "
                "value=%u mode=%u status=%u",
                (unsigned int)header.type,
                (unsigned int)header.length,
                (unsigned int)header.seq,
                (unsigned int)header.value,
                (unsigned int)header.mode,
                (unsigned int)header.status
            );

            break;
        }


        /*
         * 정상적으로 수신된 Header를 기록한다.
         */
        sim_log_packet(
            SIM_LOG_INFO,
            ctx->device_id,
            "RX_PACKET",
            &header
        );


        /*
         * =====================================
         * Payload 수신
         * =====================================
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
                DEVICE_LOG_INFO(
                    ctx->device_id,
                    "PEER_CLOSED",
                    "phase=payload "
                    "type=%u seq=%u",
                    (unsigned int)header.type,
                    (unsigned int)header.seq
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
                    "type=%u seq=%u "
                    "length=%u errno=%d message=%s",
                    (unsigned int)header.type,
                    (unsigned int)header.seq,
                    (unsigned int)header.length,
                    saved_errno,
                    strerror(saved_errno)
                );

                break;
            }


            if ((uint32_t)recv_size !=
                header.length)
            {
                DEVICE_LOG_ERROR(
                    ctx->device_id,
                    "PAYLOAD_SIZE_MISMATCH",
                    "type=%u seq=%u "
                    "received=%u expected=%u",
                    (unsigned int)header.type,
                    (unsigned int)header.seq,
                    (unsigned int)recv_size,
                    (unsigned int)header.length
                );

                break;
            }


            DEVICE_LOG_DEBUG(
                ctx->device_id,
                "RX_PAYLOAD",
                "type=%u seq=%u length=%u",
                (unsigned int)header.type,
                (unsigned int)header.seq,
                (unsigned int)header.length
            );
        }


        /*
         * =====================================
         * 장비별 Handler 호출
         * =====================================
         */
        if (ctx->packet_handler != NULL)
        {
            int handler_result;


            DEVICE_LOG_DEBUG(
                ctx->device_id,
                "HANDLER_START",
                "type=%u seq=%u",
                (unsigned int)header.type,
                (unsigned int)header.seq
            );


            handler_result =
                ctx->packet_handler(
                    ctx,
                    &header,
                    payload
                );


            if (handler_result < 0)
            {
                DEVICE_LOG_ERROR(
                    ctx->device_id,
                    "HANDLER_FAILED",
                    "type=%u seq=%u result=%d",
                    (unsigned int)header.type,
                    (unsigned int)header.seq,
                    handler_result
                );
            }
            else
            {
                DEVICE_LOG_DEBUG(
                    ctx->device_id,
                    "HANDLER_COMPLETE",
                    "type=%u seq=%u result=%d",
                    (unsigned int)header.type,
                    (unsigned int)header.seq,
                    handler_result
                );
            }
        }
        else
        {
            DEVICE_LOG_WARNING(
                ctx->device_id,
                "HANDLER_NOT_REGISTERED",
                "type=%u seq=%u",
                (unsigned int)header.type,
                (unsigned int)header.seq
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


    DEVICE_LOG_INFO(
        ctx->device_id,
        "THREAD_STARTED",
        "mode=%s port=%u",
        (ctx->is_server == 1) ?
            "server" : "client",
        (unsigned int)ctx->port
    );


    /*
     * is_server는 0 또는 1만 허용한다.
     */
    if ((ctx->is_server != 0) &&
        (ctx->is_server != 1))
    {
        DEVICE_LOG_ERROR(
            ctx->device_id,
            "INVALID_MODE",
            "is_server=%d expected=0_or_1",
            ctx->is_server
        );

        return NULL;
    }


    /*
     * =====================================
     * Server 모드 초기화
     * =====================================
     */
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
            "port=%u socket=%d",
            (unsigned int)ctx->port,
            ctx->listen_fd
        );
    }


    /*
     * Server 모드는 연결 종료 후 다시 accept한다.
     *
     * Client 모드는 연결과 통신을 한 번만 수행하고
     * 스레드를 종료한다.
     */
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
             * 클라이언트는 연결을 한 번만 시도한다.
             */
            if (ctx->is_server == 0)
            {
                DEVICE_LOG_INFO(
                    ctx->device_id,
                    "THREAD_FINISHED",
                    "reason=connect_failed"
                );

                return NULL;
            }


            /*
             * 서버는 accept에 실패해도
             * 다음 연결을 다시 기다린다.
             */
            continue;
        }


        DEVICE_LOG_INFO(
            ctx->device_id,
            "PEER_CONNECTED",
            "mode=%s socket=%d port=%u",
            (ctx->is_server == 1) ?
                "server" : "client",
            ctx->client_fd,
            (unsigned int)ctx->port
        );


        /*
         * 패킷 수신과 Handler 호출
         */
        comm_receive_packets(
            ctx
        );


        DEVICE_LOG_INFO(
            ctx->device_id,
            "PEER_DISCONNECTED",
            "socket=%d",
            ctx->client_fd
        );


        /*
         * 연결된 peer socket을 닫는다.
         */
        if (tcp_close(
                ctx->client_fd) < 0)
        {
            const int saved_errno =
                errno;

            DEVICE_LOG_WARNING(
                ctx->device_id,
                "SOCKET_CLOSE_FAILED",
                "socket=%d errno=%d message=%s",
                ctx->client_fd,
                saved_errno,
                strerror(saved_errno)
            );
        }


        ctx->client_fd = -1;


        /*
         * Client 모드는 통신을 한 번만 수행한다.
         */
        if (ctx->is_server == 0)
        {
            DEVICE_LOG_INFO(
                ctx->device_id,
                "THREAD_FINISHED",
                "reason=communication_finished"
            );

            return NULL;
        }


        /*
         * Server 모드만 while을 반복하여
         * 다음 연결을 다시 accept한다.
         */
    }


    return NULL;
}