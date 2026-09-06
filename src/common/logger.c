#include "common/logger.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>


#define LOG_MESSAGE_SIZE 1024U


static pthread_mutex_t logger_mutex =
    PTHREAD_MUTEX_INITIALIZER;

static FILE *logger_file = NULL;

static sim_log_level_t logger_minimum_level =
    SIM_LOG_INFO;


static const char *sim_log_level_name(
    sim_log_level_t level)
{
    switch (level)
    {
        case SIM_LOG_DEBUG:
            return "DEBUG";

        case SIM_LOG_INFO:
            return "INFO";

        case SIM_LOG_WARNING:
            return "WARNING";

        case SIM_LOG_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}


int sim_logger_init(
    const char *file_path,
    sim_log_level_t minimum_level)
{
    logger_minimum_level =
        minimum_level;


    if (file_path == NULL)
    {
        return 0;
    }


    logger_file = fopen(
        file_path,
        "a"
    );

    if (logger_file == NULL)
    {
        perror("fopen log file");

        return -1;
    }


    /*
     * 줄 단위 버퍼링
     */
    setvbuf(
        logger_file,
        NULL,
        _IOLBF,
        0U
    );


    return 0;
}


void sim_logger_shutdown(void)
{
    pthread_mutex_lock(
        &logger_mutex
    );


    if (logger_file != NULL)
    {
        fclose(
            logger_file
        );

        logger_file = NULL;
    }


    pthread_mutex_unlock(
        &logger_mutex
    );
}


void sim_log_write(
    sim_log_level_t level,
    int device_id,
    const char *event,
    const char *format,
    ...)
{
    struct timespec current_time;

    struct tm local_time;

    char time_text[64];

    char message[
        LOG_MESSAGE_SIZE
    ];

    va_list arguments;


    if (level <
        logger_minimum_level)
    {
        return;
    }


    if ((event == NULL) ||
        (format == NULL))
    {
        return;
    }


    if (clock_gettime(
            CLOCK_REALTIME,
            &current_time) < 0)
    {
        return;
    }


    if (localtime_r(
            &current_time.tv_sec,
            &local_time) == NULL)
    {
        return;
    }


    strftime(
        time_text,
        sizeof(time_text),
        "%Y-%m-%d %H:%M:%S",
        &local_time
    );


    va_start(
        arguments,
        format
    );

    vsnprintf(
        message,
        sizeof(message),
        format,
        arguments
    );

    va_end(
        arguments
    );


    pthread_mutex_lock(
        &logger_mutex
    );


    fprintf(
        stderr,
        "%s.%03ld %-7s device=%d event=%s %s\n",
        time_text,
        current_time.tv_nsec / 1000000L,
        sim_log_level_name(level),
        device_id,
        event,
        message
    );


    if (logger_file != NULL)
    {
        fprintf(
            logger_file,
            "%s.%03ld %-7s device=%d event=%s %s\n",
            time_text,
            current_time.tv_nsec / 1000000L,
            sim_log_level_name(level),
            device_id,
            event,
            message
        );
    }


    pthread_mutex_unlock(
        &logger_mutex
    );
}


void sim_log_packet(
    sim_log_level_t level,
    int device_id,
    const char *direction,
    const packet_header_t *header)
{
    if ((direction == NULL) ||
        (header == NULL))
    {
        return;
    }


    sim_log_write(
        level,
        device_id,
        direction,
        "type=%u length=%u seq=%u "
        "value=%u mode=%u status=%u",
        (unsigned int)header->type,
        (unsigned int)header->length,
        (unsigned int)header->seq,
        (unsigned int)header->value,
        (unsigned int)header->mode,
        (unsigned int)header->status
    );
}