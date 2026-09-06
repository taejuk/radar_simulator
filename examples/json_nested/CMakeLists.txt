cmake_minimum_required(VERSION 3.16)


project(
    json_nested_example
    LANGUAGES C
)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# =========================================================
# Local cJSON
# =========================================================

set(
    CJSON_DIR
    ${CMAKE_CURRENT_LIST_DIR}/../../external/cJSON
)


set(
    REQUIRED_CJSON_FILES

    ${CJSON_DIR}/cJSON.c
    ${CJSON_DIR}/cJSON.h
    ${CJSON_DIR}/LICENSE
)


foreach(
    REQUIRED_FILE
    IN LISTS REQUIRED_CJSON_FILES
)
    if(NOT EXISTS "${REQUIRED_FILE}")
        message(
            FATAL_ERROR
            "Required local cJSON file is missing:\n"
            "  ${REQUIRED_FILE}\n"
        )
    endif()
endforeach()


add_library(
    example_cjson
    STATIC

    ${CJSON_DIR}/cJSON.c
)


target_include_directories(
    example_cjson

    PUBLIC
        ${CJSON_DIR}
)


# =========================================================
# JSON nested-structure example
# =========================================================

add_executable(
    json_nested_example

    json_nested_example.c
)


target_compile_definitions(
    json_nested_example

    PRIVATE
        EXAMPLE_JSON_PATH="${CMAKE_CURRENT_LIST_DIR}/device_config.json"
)


target_compile_options(
    json_nested_example

    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)


target_link_libraries(
    json_nested_example

    PRIVATE
        example_cjson
)


if(UNIX)
    target_link_libraries(
        json_nested_example
        PRIVATE
    )
endif()