#include "common/packet.h"

#include "config.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>


int internal_msg_header_hton(
    const InternalMsgHeader_t *host_header,
    InternalMsgHeader_t *network_header)
{
    if ((host_header == NULL) ||
        (network_header == NULL))
    {
        return -1;
    }


    network_header->msgType =
        htons(
            host_header->msgType
        );

    network_header->msgSize =
        htonl(
            host_header->msgSize
        );

    network_header->msgSec =
        htonl(
            host_header->msgSec
        );

    network_header->msgNSec =
        htonl(
            host_header->msgNSec
        );

    /*
     * 1-byte 값은 byte order 변환이 필요 없다.
     */
    network_header->srcId =
        host_header->srcId;

    network_header->destId =
        host_header->destId;


    return 0;
}


int internal_msg_header_ntoh(
    const InternalMsgHeader_t *network_header,
    InternalMsgHeader_t *host_header)
{
    if ((network_header == NULL) ||
        (host_header == NULL))
    {
        return -1;
    }


    host_header->msgType =
        ntohs(
            network_header->msgType
        );

    host_header->msgSize =
        ntohl(
            network_header->msgSize
        );

    host_header->msgSec =
        ntohl(
            network_header->msgSec
        );

    host_header->msgNSec =
        ntohl(
            network_header->msgNSec
        );

    host_header->srcId =
        network_header->srcId;

    host_header->destId =
        network_header->destId;


    return 0;
}


int internal_msg_header_validate(
    const InternalMsgHeader_t *header)
{
    if (header == NULL)
    {
        return -1;
    }


    /*
     * msgSize는 Header 뒤에 오는
     * payload의 바이트 크기이다.
     */
    if (header->msgSize >
        MAX_PAYLOAD_SIZE)
    {
        return -1;
    }


    /*
     * nanosecond의 유효 범위
     */
    if (header->msgNSec >
        999999999U)
    {
        return -1;
    }


    /*
     * 현재 시뮬레이터에서 처리하는 메시지 타입
     */
    switch (header->msgType)
    {
        case PACKET_START:
        case PACKET_STATUS:
        case PACKET_CONTROL:
            return 0;

        default:
            return -1;
    }
}


int internal_msg_header_set_realtime(
    InternalMsgHeader_t *header)
{
    struct timespec current_time;

    uint64_t seconds;


    if (header == NULL)
    {
        return -1;
    }


    if (clock_gettime(
            CLOCK_REALTIME,
            &current_time) < 0)
    {
        return -1;
    }


    /*
     * tv_sec가 uint32_t 범위인지 검사한다.
     *
     * 음수인 경우 uint64_t 변환 후 큰 값이 되므로
     * 이 검사에서 함께 걸러진다.
     */
    seconds =
        (uint64_t)current_time.tv_sec;


    if (seconds >
        UINT32_MAX)
    {
        return -1;
    }


    if ((current_time.tv_nsec < 0L) ||
        (current_time.tv_nsec >
         999999999L))
    {
        return -1;
    }


    header->msgSec =
        (uint32_t)seconds;

    header->msgNSec =
        (uint32_t)current_time.tv_nsec;


    return 0;
}