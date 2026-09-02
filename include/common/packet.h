#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include <stdint.h>


#define PACKET_HEADER_SIZE  16U

typedef enum
{
    PACKET_START   = 1,
    PACKET_STATUS  = 2,
    PACKET_CONTROL = 3

} packet_type_t;


/*
 * Wire protocol header
 *
 * 실제 필드 이름은 프로토콜 규격에 맞게 변경.
 */
typedef struct
{
    uint16_t type;

    uint32_t length;

    uint32_t seq;

    uint32_t value;

    uint8_t mode;

    uint8_t status;

} packet_header_t;

int packet_header_validate(
    const packet_header_t *header
);


#endif