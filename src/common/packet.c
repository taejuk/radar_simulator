#include "common/packet.h"


int packet_header_validate(
    const packet_header_t *header)
{
    if (header == NULL)
    {
        return -1;
    }


    /*
     * length는 payload 길이라고 가정.
     */
    if (header->length > MAX_PAYLOAD_SIZE)
    {
        return -1;
    }


    switch (header->type)
    {
        case PACKET_START:
        case PACKET_STATUS:
        case PACKET_CONTROL:

            return 0;


        default:

            return -1;
    }
}