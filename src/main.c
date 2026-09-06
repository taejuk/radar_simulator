#include "config.h"

#include "common/comm_thread.h"
#include "common/logger.h"

#include "device/device_handler.h"

#include "gui/device_gui_state.h"
#include "gui/gui_main.h"

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>


int main(void)
{
    pthread_t threads[
        DEVICE_COUNT
    ];


    device_gui_state_t gui_states[
        DEVICE_COUNT
    ];


    device_context_t devices[
        DEVICE_COUNT
    ] =
    {
        {
            .device_id = 1,
            .port = DEVICE_A_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        },

        {
            .device_id = 2,
            .port = DEVICE_B_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        },

        {
            .device_id = 3,
            .port = DEVICE_C_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        },

        {
            .device_id = 4,
            .port = DEVICE_D_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        },

        {
            .device_id = 5,
            .port = DEVICE_E_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        },

        {
            .device_id = 6,
            .port = DEVICE_F_PORT,
            .is_server = 0,
            .listen_fd = -1,
            .client_fd = -1,
            .state = 0,
            .gui_state = NULL,
            .packet_handler = device_handler
        }
    };


    size_t initialized_count =
        0U;

    size_t started_count =
        0U;

    int gui_ret =
        -1;

    int exit_code =
        1;


    if (sim_logger_init(
            "simulator.log",
            SIM_LOG_DEBUG) < 0)
    {
        fprintf(
            stderr,
            "sim_logger_init failed\n"
        );

        return 1;
    }


    /*
     * Device마다 독립적인 GUI 공유 상태를 생성한다.
     */
    for (size_t i = 0U;
         i < DEVICE_COUNT;
         ++i)
    {
        if (device_gui_state_init(
                &gui_states[i]) < 0)
        {
            fprintf(
                stderr,
                "device_gui_state_init failed: "
                "device=%d\n",
                devices[i].device_id
            );

            goto cleanup;
        }


        devices[i].gui_state =
            &gui_states[i];

        initialized_count +=
            1U;
    }


    /*
     * Device마다 통신 스레드를 하나씩 생성한다.
     */
    for (size_t i = 0U;
         i < DEVICE_COUNT;
         ++i)
    {
        const int ret =
            pthread_create(
                &threads[i],
                NULL,
                comm_thread,
                &devices[i]
            );


        if (ret != 0)
        {
            fprintf(
                stderr,
                "pthread_create failed: "
                "device=%d error=%d\n",
                devices[i].device_id,
                ret
            );

            goto cleanup;
        }


        started_count +=
            1U;
    }


    /*
     * GLFW/OpenGL/ImGui는 메인 스레드에서 실행한다.
     */
    gui_ret = gui_run(
        gui_states,
        DEVICE_COUNT
    );


    if (gui_ret == 0)
    {
        exit_code =
            0;
    }


cleanup:

    /*
     * GUI 응답을 기다리는 모든 Device thread를 깨운다.
     */
    for (size_t i = 0U;
         i < initialized_count;
         ++i)
    {
        device_gui_request_shutdown(
            &gui_states[i]
        );
    }


    /*
     * recv()에서 대기 중인 socket을 깨운다.
     */
    for (size_t i = 0U;
         i < started_count;
         ++i)
    {
        if (devices[i].client_fd >= 0)
        {
            (void)shutdown(
                devices[i].client_fd,
                SHUT_RDWR
            );
        }
    }


    for (size_t i = 0U;
         i < started_count;
         ++i)
    {
        const int ret =
            pthread_join(
                threads[i],
                NULL
            );


        if (ret != 0)
        {
            fprintf(
                stderr,
                "pthread_join failed: "
                "device=%d error=%d\n",
                devices[i].device_id,
                ret
            );

            exit_code =
                1;
        }
    }


    for (size_t i = 0U;
         i < initialized_count;
         ++i)
    {
        device_gui_state_destroy(
            &gui_states[i]
        );
    }


    sim_logger_shutdown();


    return exit_code;
}