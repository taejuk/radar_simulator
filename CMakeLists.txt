cmake_minimum_required(VERSION 3.16)

project(tcp_simulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

find_package(Threads REQUIRED)

# =========================================================
# Configuration
# =========================================================

set(DEVICE_COUNT 6)

set(DEVICE_A_PORT 5001)
set(DEVICE_B_PORT 5002)
set(DEVICE_C_PORT 5003)
set(DEVICE_D_PORT 5004)
set(DEVICE_E_PORT 5005)
set(DEVICE_F_PORT 5006)

set(MAX_PAYLOAD_SIZE 4096)
set(LISTEN_BACKLOG 1)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/config.h
    @ONLY
)


# 
# executable files
#


add_executable(simulator

    src/main.c

    src/common/tcp.c
    src/common/packet.c
    src/common/comm_thread.c

    src/device/device_a.c
    src/device/device_b.c
    src/device/device_c.c
    src/device/device_d.c
    src/device/device_e.c
    src/device/device_f.c

    src/handler/device_a_handler.c
    src/handler/device_b_handler.c
    src/handler/device_c_handler.c
    src/handler/device_d_handler.c
    src/handler/device_e_handler.c
    src/handler/device_f_handler.c
)

target_include_directories(simulator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_compile_options(simulator
    PRIVATE
        -Wall
        -Wextra
        -Werror
)

target_link_libraries(simulator
    PRIVATE
        Threads::Threads
)

# test 파일 관련 추가할 것들
add_executable(
    test_client

    tests/test_client.c

    src/common/tcp.c
)

target_include_directories(
    test_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)