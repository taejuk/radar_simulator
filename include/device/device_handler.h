#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include "common/comm_thread.h"


/*
 * 현재 A~F 공통 handler다.
 *
 * Device별 GUI가 서로 다른 payload 구조체를 만들지만
 * handler에서는 byte buffer로 받아 전송한다.
 */
int device_handler(
    device_context_t *ctx,
    const InternalMsgHeader_t *header,
    const uint8_t *payload
);


#endif