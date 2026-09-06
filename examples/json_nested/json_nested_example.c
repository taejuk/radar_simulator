#include "cJSON.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef EXAMPLE_JSON_PATH
#define EXAMPLE_JSON_PATH \
    "device_config.json"
#endif


#define JSON_FILE_MAX_SIZE \
    (64U * 1024U)


/*
 * 전송 시간 관련 구조체
 */
typedef struct packet_timestamp
{
    uint32_t sec;
    uint32_t nsec;
} packet_timestamp_t;


/*
 * Built-In Test 결과
 *
 * 0: 정상
 * 1: 비정상
 */
typedef struct built_in_test
{
    uint8_t power_supply;
    uint8_t transmitter;
    uint8_t receiver;
} built_in_test_t;


/*
 * 온도 정보
 */
typedef struct temperature_state
{
    float cpu_celsius;
    float rf_celsius;
} temperature_state_t;


/*
 * 동작 상태
 */
typedef struct operation_state
{
    uint8_t mode;
    uint8_t status;
} operation_state_t;


/*
 * 다른 구조체를 포함하는 최종 구조체
 */
typedef struct radar_packet_config
{
    uint16_t message_type;

    packet_timestamp_t timestamp;

    built_in_test_t built_in_test;

    temperature_state_t temperature;

    operation_state_t operation;
} radar_packet_config_t;


/*
 * JSON 파일 전체를 문자열로 읽는다.
 */
static int read_text_file(
    const char *path,
    char **text)
{
    FILE *file =
        NULL;

    char *buffer =
        NULL;

    long file_size;

    size_t read_size;

    int result =
        -1;


    if ((path == NULL) ||
        (text == NULL))
    {
        return -1;
    }


    *text =
        NULL;


    file = fopen(
        path,
        "rb"
    );


    if (file == NULL)
    {
        perror(path);

        goto cleanup;
    }


    if (fseek(
            file,
            0L,
            SEEK_END) != 0)
    {
        goto cleanup;
    }


    file_size =
        ftell(file);


    if ((file_size <= 0L) ||
        ((unsigned long)file_size >
         JSON_FILE_MAX_SIZE))
    {
        goto cleanup;
    }


    if (fseek(
            file,
            0L,
            SEEK_SET) != 0)
    {
        goto cleanup;
    }


    buffer = malloc(
        (size_t)file_size + 1U
    );


    if (buffer == NULL)
    {
        goto cleanup;
    }


    read_size = fread(
        buffer,
        1U,
        (size_t)file_size,
        file
    );


    if (read_size !=
        (size_t)file_size)
    {
        goto cleanup;
    }


    buffer[read_size] =
        '\0';


    *text =
        buffer;

    buffer =
        NULL;

    result =
        0;


cleanup:

    free(buffer);


    if (file != NULL)
    {
        (void)fclose(file);
    }


    return result;
}


/*
 * 필수 중첩 객체를 가져온다.
 */
static const cJSON *get_required_object(
    const cJSON *parent,
    const char *name)
{
    const cJSON *item =
        cJSON_GetObjectItemCaseSensitive(
            parent,
            name
        );


    if (!cJSON_IsObject(item))
    {
        return NULL;
    }


    return item;
}


/*
 * JSON 정수 필드를 읽고 범위를 검사한다.
 */
static int get_required_uint(
    const cJSON *parent,
    const char *name,
    uint64_t minimum,
    uint64_t maximum,
    uint64_t *result)
{
    const cJSON *item;

    double number;

    uint64_t integer;


    if ((parent == NULL) ||
        (name == NULL) ||
        (result == NULL))
    {
        return -1;
    }


    item =
        cJSON_GetObjectItemCaseSensitive(
            parent,
            name
        );


    if (!cJSON_IsNumber(item))
    {
        return -1;
    }


    number =
        item->valuedouble;


    if ((!isfinite(number)) ||
        (number < (double)minimum) ||
        (number > (double)maximum))
    {
        return -1;
    }


    integer =
        (uint64_t)number;


    /*
     * 정수 필드에 1.5와 같은 실수는 허용하지 않는다.
     */
    if ((double)integer != number)
    {
        return -1;
    }


    *result =
        integer;


    return 0;
}


/*
 * JSON 실수 필드를 읽고 범위를 검사한다.
 */
static int get_required_float(
    const cJSON *parent,
    const char *name,
    double minimum,
    double maximum,
    float *result)
{
    const cJSON *item;

    double number;


    if ((parent == NULL) ||
        (name == NULL) ||
        (result == NULL))
    {
        return -1;
    }


    item =
        cJSON_GetObjectItemCaseSensitive(
            parent,
            name
        );


    if (!cJSON_IsNumber(item))
    {
        return -1;
    }


    number =
        item->valuedouble;


    if ((!isfinite(number)) ||
        (number < minimum) ||
        (number > maximum))
    {
        return -1;
    }


    *result =
        (float)number;


    return 0;
}


/*
 * JSON 파일을 읽어서 중첩 구조체에 저장한다.
 *
 * 모든 필드가 성공적으로 검증된 경우에만
 * 최종 구조체에 값을 복사한다.
 *
 * 하나라도 실패하면 config는 모두 0이다.
 */
static int load_radar_packet_config(
    const char *path,
    radar_packet_config_t *config)
{
    char *text =
        NULL;

    cJSON *root =
        NULL;

    const cJSON *timestamp;

    const cJSON *built_in_test;

    const cJSON *temperature;

    const cJSON *operation;


    radar_packet_config_t parsed =
    {
        0
    };


    uint64_t integer;

    int result =
        -1;


    if ((path == NULL) ||
        (config == NULL))
    {
        return -1;
    }


    /*
     * JSON 처리 실패 시 사용할 기본값
     */
    memset(
        config,
        0,
        sizeof(*config)
    );


    if (read_text_file(
            path,
            &text) < 0)
    {
        goto cleanup;
    }


    root =
        cJSON_Parse(text);


    if (!cJSON_IsObject(root))
    {
        fprintf(
            stderr,
            "JSON root must be an object\n"
        );

        goto cleanup;
    }


    /*
     * 중첩 객체를 먼저 가져온다.
     */
    timestamp =
        get_required_object(
            root,
            "timestamp"
        );


    built_in_test =
        get_required_object(
            root,
            "built_in_test"
        );


    temperature =
        get_required_object(
            root,
            "temperature"
        );


    operation =
        get_required_object(
            root,
            "operation"
        );


    if ((timestamp == NULL) ||
        (built_in_test == NULL) ||
        (temperature == NULL) ||
        (operation == NULL))
    {
        fprintf(
            stderr,
            "A required nested object is missing\n"
        );

        goto cleanup;
    }


    /*
     * message_type
     */
    if (get_required_uint(
            root,
            "message_type",
            1U,
            3U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.message_type =
        (uint16_t)integer;


    /*
     * timestamp.sec
     */
    if (get_required_uint(
            timestamp,
            "sec",
            0U,
            UINT32_MAX,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.timestamp.sec =
        (uint32_t)integer;


    /*
     * timestamp.nsec
     */
    if (get_required_uint(
            timestamp,
            "nsec",
            0U,
            999999999U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.timestamp.nsec =
        (uint32_t)integer;


    /*
     * BIT: power_supply
     */
    if (get_required_uint(
            built_in_test,
            "power_supply",
            0U,
            1U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.built_in_test.power_supply =
        (uint8_t)integer;


    /*
     * BIT: transmitter
     */
    if (get_required_uint(
            built_in_test,
            "transmitter",
            0U,
            1U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.built_in_test.transmitter =
        (uint8_t)integer;


    /*
     * BIT: receiver
     */
    if (get_required_uint(
            built_in_test,
            "receiver",
            0U,
            1U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.built_in_test.receiver =
        (uint8_t)integer;


    /*
     * CPU 온도
     *
     * 예제 범위: -100 ~ 200도
     */
    if (get_required_float(
            temperature,
            "cpu_celsius",
            -100.0,
            200.0,
            &parsed.temperature.cpu_celsius) < 0)
    {
        goto invalid_field;
    }


    /*
     * RF 온도
     */
    if (get_required_float(
            temperature,
            "rf_celsius",
            -100.0,
            200.0,
            &parsed.temperature.rf_celsius) < 0)
    {
        goto invalid_field;
    }


    /*
     * mode
     */
    if (get_required_uint(
            operation,
            "mode",
            0U,
            1U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.operation.mode =
        (uint8_t)integer;


    /*
     * status
     */
    if (get_required_uint(
            operation,
            "status",
            0U,
            1U,
            &integer) < 0)
    {
        goto invalid_field;
    }


    parsed.operation.status =
        (uint8_t)integer;


    /*
     * 모든 필드의 파싱과 검증이 성공한 후에만 복사한다.
     */
    *config =
        parsed;


    result =
        0;


    goto cleanup;


invalid_field:

    fprintf(
        stderr,
        "A JSON field is missing, "
        "invalid or out of range\n"
    );


cleanup:

    cJSON_Delete(root);

    free(text);


    return result;
}


/*
 * 구조체에 저장된 결과를 출력한다.
 */
static void print_radar_packet_config(
    const radar_packet_config_t *config)
{
    printf(
        "message_type=%u\n"
        "timestamp.sec=%u\n"
        "timestamp.nsec=%u\n"
        "built_in_test.power_supply=%u\n"
        "built_in_test.transmitter=%u\n"
        "built_in_test.receiver=%u\n"
        "temperature.cpu_celsius=%.2f\n"
        "temperature.rf_celsius=%.2f\n"
        "operation.mode=%u\n"
        "operation.status=%u\n",

        (unsigned int)
            config->message_type,

        (unsigned int)
            config->timestamp.sec,

        (unsigned int)
            config->timestamp.nsec,

        (unsigned int)
            config->built_in_test.power_supply,

        (unsigned int)
            config->built_in_test.transmitter,

        (unsigned int)
            config->built_in_test.receiver,

        (double)
            config->temperature.cpu_celsius,

        (double)
            config->temperature.rf_celsius,

        (unsigned int)
            config->operation.mode,

        (unsigned int)
            config->operation.status
    );
}


int main(
    int argc,
    char **argv)
{
    radar_packet_config_t config;


    const char *json_path =
        EXAMPLE_JSON_PATH;


    if (argc > 1)
    {
        json_path =
            argv[1];
    }


    if (load_radar_packet_config(
            json_path,
            &config) < 0)
    {
        fprintf(
            stderr,
            "JSON load failed; "
            "zero-filled structure follows\n"
        );


        print_radar_packet_config(
            &config
        );


        return EXIT_FAILURE;
    }


    printf(
        "JSON load succeeded: %s\n",
        json_path
    );


    print_radar_packet_config(
        &config
    );


    return EXIT_SUCCESS;
}