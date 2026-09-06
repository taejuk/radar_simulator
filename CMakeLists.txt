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
# Required project files
# =========================================================

set(
    REQUIRED_GUI_FILES

    ${CMAKE_CURRENT_SOURCE_DIR}/include/gui/gui_main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gui/device_a_gui_state.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/device_a_gui_state.c
)


foreach(REQUIRED_FILE IN LISTS REQUIRED_GUI_FILES)
    if(NOT EXISTS "${REQUIRED_FILE}")
        message(
            FATAL_ERROR
            "Required GUI file is missing:\n"
            "  ${REQUIRED_FILE}\n"
        )
    endif()
endforeach()


# =========================================================
# System dependencies
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
        "Install them on Ubuntu with:\n"
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
        "Install them on Ubuntu with:\n"
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
        "CMake found OpenGL, but no usable OpenGL target exists."
    )
endif()


# =========================================================
# Dear ImGui
#
# external/imgui가 있으면 해당 소스를 사용한다.
# 없으면 공식 GitHub 저장소에서 고정 버전을 받는다.
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


# =========================================================
# Common source lists
# =========================================================

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
# Main simulator
#
# 현재 main.c는 Device A만 사용하므로
# Device A 관련 소스만 포함한다.
# =========================================================

add_executable(
    simulator

    src/main.c

    ${NETWORK_SOURCES}
    src/common/comm_thread.c
    src/common/logger.c

    src/device/device_a.c
    src/handler/device_a_handler.c

    src/gui/device_a_gui_state.c
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
# Test client
# =========================================================

add_executable(
    test_client

    tests/test_client.c
    src/common/tcp.c
    src/common/packet.c
)


target_include_directories(
    test_client

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)


target_compile_options(
    test_client

    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)


# =========================================================
# Test server
# =========================================================

add_executable(
    test_server

    tests/test_server.c
    ${NETWORK_SOURCES}
)


target_include_directories(
    test_server

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)


target_compile_options(
    test_server

    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)