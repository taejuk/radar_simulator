cmake_minimum_required(VERSION 3.16)

project(
    tcp_simulator
    LANGUAGES C CXX
)


# =========================================================
# Language settings
# =========================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# =========================================================
# Simulator configuration
# =========================================================

set(DEVICE_COUNT 6)

set(SERVER_IP "127.0.0.1")

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


# =========================================================
# Dependencies
# =========================================================

find_package(
    Threads REQUIRED
)

find_package(
    OpenGL QUIET
)

if(NOT OpenGL_FOUND)
    message(
        FATAL_ERROR
        "OpenGL development files were not found.\n"
        "Install them with:\n"
        "  sudo apt install libgl-dev libglx-dev "
        "libopengl-dev libglfw3-dev\n"
    )
endif()


find_package(
    glfw3 3.3 CONFIG QUIET
)

if(TARGET glfw)
    set(GLFW_TARGET glfw)

elseif(TARGET glfw3::glfw)
    set(GLFW_TARGET glfw3::glfw)

else()
    message(
        FATAL_ERROR
        "GLFW development files were not found.\n"
        "Install them with:\n"
        "  sudo apt install libglfw3-dev\n"
    )
endif()


if(TARGET OpenGL::GL)
    set(OPENGL_TARGET OpenGL::GL)

elseif(TARGET OpenGL::OpenGL)
    set(OPENGL_TARGET OpenGL::OpenGL)

else()
    message(
        FATAL_ERROR
        "CMake found OpenGL, but no usable target exists."
    )
endif()


# =========================================================
# Dear ImGui
# =========================================================

set(
    LOCAL_IMGUI_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
)

if(EXISTS "${LOCAL_IMGUI_DIR}/imgui.cpp")
    set(
        IMGUI_DIR
        ${LOCAL_IMGUI_DIR}
    )

    message(
        STATUS
        "Using local Dear ImGui: ${IMGUI_DIR}"
    )
else()
    include(FetchContent)

    FetchContent_Declare(
        imgui

        GIT_REPOSITORY
            https://github.com/ocornut/imgui.git

        GIT_TAG
            v1.92.9b

        GIT_SHALLOW
            TRUE
    )

    FetchContent_MakeAvailable(
        imgui
    )

    set(
        IMGUI_DIR
        ${imgui_SOURCE_DIR}
    )

    message(
        STATUS
        "Using fetched Dear ImGui: ${IMGUI_DIR}"
    )
endif()


set(
    NETWORK_SOURCES

    src/common/tcp.c
    src/common/packet.c
)


set(
    IMGUI_SOURCES

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)


# =========================================================
# Simulator
# =========================================================

add_executable(
    simulator

    src/main.c

    ${NETWORK_SOURCES}

    src/common/comm_thread.c
    src/common/logger.c

    src/device/device_handler.c

    src/gui/device_gui_state.c
    src/gui/gui_main.cpp

    ${IMGUI_SOURCES}
)


target_include_directories(
    simulator

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated

        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)


target_link_libraries(
    simulator

    PRIVATE
        Threads::Threads
        ${GLFW_TARGET}
        ${OPENGL_TARGET}
)


target_compile_options(
    simulator

    PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
            -Wall
            -Wextra
            -Wpedantic
        >

        $<$<COMPILE_LANGUAGE:CXX>:
            -Wall
            -Wextra
            -Wpedantic
        >
)




# =========================================================
# Device A~F exchange tests
# =========================================================

function(
    add_device_exchange_test
    TARGET_NAME
    TEST_SOURCE
)
    add_executable(
        ${TARGET_NAME}

        ${TEST_SOURCE}
        tests/test_exchange_common.c

        ${NETWORK_SOURCES}
    )

    target_include_directories(
        ${TARGET_NAME}

        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
            ${CMAKE_CURRENT_BINARY_DIR}/generated
    )

    target_compile_options(
        ${TARGET_NAME}

        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endfunction()

add_device_exchange_test(
    test_client_a
    tests/test_client_a.c
)


add_device_exchange_test(
    test_client_b
    tests/test_client_b.c
)

add_device_exchange_test(
    test_client_c
    tests/test_client_c.c
)

add_device_exchange_test(
    test_client_d
    tests/test_client_d.c
)

add_device_exchange_test(
    test_client_e
    tests/test_client_e.c
)

add_device_exchange_test(
    test_client_f
    tests/test_client_f.c
)

add_device_exchange_test(
    test_server_a
    tests/test_server_a.c
)

add_device_exchange_test(
    test_server_b
    tests/test_server_b.c
)

add_device_exchange_test(
    test_server_c
    tests/test_server_c.c
)

add_device_exchange_test(
    test_server_d
    tests/test_server_d.c
)

add_device_exchange_test(
    test_server_e
    tests/test_server_e.c
)

add_device_exchange_test(
    test_server_f
    tests/test_server_f.c
)