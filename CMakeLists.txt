cmake_minimum_required(VERSION 3.16)

project(tcp_simulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

find_package(Threads REQUIRED)

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
)