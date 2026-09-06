#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define INTERNAL_MSG_HEADER_SIZE 16U


typedef enum
{
    PACKET_START   = 1,
    PACKET_STATUS  = 2,
    PACKET_CONTROL = 3

} packet_type_t;


/*
 * Internal message header
 *
 * Wire size:
 * 2 + 4 + 4 + 4 + 1 + 1 = 16 bytes
 */
typedef struct __attribute__((packed))
{
    uint16_t msgType;

    uint32_t msgSize;

    uint32_t msgSec;

    uint32_t msgNSec;

    uint8_t srcId;

    uint8_t destId;

} InternalMsgHeader_t;



/*
 * Host byte order Header를
 * Big Endian Header로 변환한다.
 */
int internal_msg_header_hton(
    const InternalMsgHeader_t *host_header,
    InternalMsgHeader_t *network_header
);


/*
 * Big Endian Header를
 * Host byte order Header로 변환한다.
 */
int internal_msg_header_ntoh(
    const InternalMsgHeader_t *network_header,
    InternalMsgHeader_t *host_header
);


/*
 * Host byte order로 변환된 Header를 검증한다.
 */
int internal_msg_header_validate(
    const InternalMsgHeader_t *header
);


/*
 * CLOCK_REALTIME으로 msgSec/msgNSec을 설정한다.
 */
int internal_msg_header_set_realtime(
    InternalMsgHeader_t *header
);


#ifdef __cplusplus
}
#endif


#endif