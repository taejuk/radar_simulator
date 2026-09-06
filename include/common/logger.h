#ifndef LOGGER_H
#define LOGGER_H

#include "common/packet.h"

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    SIM_LOG_DEBUG = 0,
    SIM_LOG_INFO,
    SIM_LOG_WARNING,
    SIM_LOG_ERROR

} sim_log_level_t;


/*
 * file_path가 NULL이면 파일에는 기록하지 않고
 * 터미널에만 출력한다.
 */
int sim_logger_init(
    const char *file_path,
    sim_log_level_t minimum_level
);


void sim_logger_shutdown(void);


void sim_log_write(
    sim_log_level_t level,
    int device_id,
    const char *event,
    const char *format,
    ...
);


void sim_log_packet(
    sim_log_level_t level,
    int device_id,
    const char *direction,
    const packet_header_t *header
);


#define DEVICE_LOG_DEBUG(device_id, event, ...) \
    sim_log_write(                              \
        SIM_LOG_DEBUG,                          \
        device_id,                              \
        event,                                  \
        __VA_ARGS__                             \
    )

#define DEVICE_LOG_INFO(device_id, event, ...)  \
    sim_log_write(                              \
        SIM_LOG_INFO,                           \
        device_id,                              \
        event,                                  \
        __VA_ARGS__                             \
    )

#define DEVICE_LOG_WARNING(device_id, event, ...) \
    sim_log_write(                                \
        SIM_LOG_WARNING,                          \
        device_id,                                \
        event,                                    \
        __VA_ARGS__                               \
    )

#define DEVICE_LOG_ERROR(device_id, event, ...) \
    sim_log_write(                              \
        SIM_LOG_ERROR,                          \
        device_id,                              \
        event,                                  \
        __VA_ARGS__                             \
    )


#ifdef __cplusplus
}
#endif


#endif