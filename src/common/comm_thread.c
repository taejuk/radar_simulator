#include "common/comm_thread.h"

#include "common/packet.h"
#include "common/tcp.h"

#include <stdint.h>
#include <stdio.h>


void *comm_thread(
    void *arg)
{
    device_context_t *ctx =
        (device_context_t *)arg;


    /*
     * =================================
     * TCP Server 초기화
     * =================================
     */

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


    /*
     * =================================
     * Connection Loop
     * =================================
     */

    while (1)
    {
        ctx->client_fd =
            tcp_accept(ctx->listen_fd);


        if (ctx->client_fd < 0)
        {
            fprintf(
                stderr,
                "[Device %d] accept failed\n",
                ctx->device_id
            );

            continue;
        }


        printf(
            "[Device %d] client connected\n",
            ctx->device_id
        );


        /*
         * =============================
         * Packet Loop
         * =============================
         */

        while (1)
        {
            packet_header_t header;

            uint8_t payload[
                MAX_PAYLOAD_SIZE
            ];

            ssize_t recv_size;


            /*
             * -------------------------
             * 1. Header 수신
             * -------------------------
             */

            recv_size = tcp_recv_exact(
                ctx->client_fd,
                &header,
                sizeof(header)
            );


            if (recv_size == 0)
            {
                /*
                 * 상대방 disconnect
                 */
                break;
            }


            if (recv_size < 0)
            {
                fprintf(
                    stderr,
                    "[Device %d] "
                    "header recv failed\n",
                    ctx->device_id
                );

                break;
            }


            /*
             * 프로토콜이 Network Byte Order이면
             * 여기에서 ntohs()/ntohl() 변환 필요.
             *
             * 현재는 실제 프로토콜 정보를 모르므로
             * 변환하지 않음.
             */


            /*
             * -------------------------
             * 2. Header 검증
             * -------------------------
             */

            if (packet_header_validate(
                    &header) < 0)
            {
                fprintf(
                    stderr,
                    "[Device %d] "
                    "invalid header\n",
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
             * -------------------------
             * 3. Payload 수신
             * -------------------------
             */

            if (header.length > 0U)
            {
                recv_size =
                    tcp_recv_exact(
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
                        "[Device %d] "
                        "payload recv failed\n",
                        ctx->device_id
                    );

                    break;
                }
            }


            /*
             * 여기까지 왔으면
             *
             * Header + Payload
             *
             * 하나의 Packet 수신 완료.
             */


            /*
             * -------------------------
             * 4. Device Handler 호출
             * -------------------------
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
                        "[Device %d] "
                        "packet handler failed\n",
                        ctx->device_id
                    );
                }
            }
        }


        /*
         * =============================
         * Client 연결 종료
         * =============================
         */

        printf(
            "[Device %d] client disconnected\n",
            ctx->device_id
        );


        tcp_close(
            ctx->client_fd
        );


        ctx->client_fd = -1;


        /*
         * 다시 accept()로 이동
         */
    }


    return NULL;
}