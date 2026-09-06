#include "config.h"

#include "common/comm_thread.h"
#include "common/logger.h"

#include "device/device_handler.h"

#if ENABLE_GUI
#include "gui/device_gui_state.h"
#include "gui/gui_main.h"
#endif

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>


int main(void)
{
    pthread_t threads[
        DEVICE_COUNT
    ];


#if ENABLE_GUI

    device_gui_state_t gui_states[
        DEVICE_COUNT
    ];

    size_t initialized_count =
        0U;

#endif


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


    size_t started_count =
        0U;

    int stop_threads =
        0;

    int exit_code =
        0;


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


#if ENABLE_GUI

    /*
     * GUI 모드에서만 Device별 GUI 상태를 생성한다.
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

            exit_code =
                1;

            stop_threads =
                1;

            goto cleanup;
        }


        devices[i].gui_state =
            &gui_states[i];

        initialized_count +=
            1U;
    }

#else

    printf(
        "[Simulator] GUI disabled: "
        "default response packets are enabled\n"
    );

#endif


    /*
     * Device 통신 스레드 생성
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

            exit_code =
                1;

            stop_threads =
                1;

            goto cleanup;
        }


        started_count +=
            1U;
    }


#if ENABLE_GUI

    /*
     * GLFW/OpenGL/ImGui는 메인 스레드에서 실행한다.
     */
    if (gui_run(
            gui_states,
            DEVICE_COUNT) < 0)
    {
        exit_code =
            1;
    }


    stop_threads =
        1;

#endif


cleanup:


#if ENABLE_GUI

    if (stop_threads != 0)
    {
        /*
         * GUI 응답 대기 중인 스레드를 깨운다.
         */
        for (size_t i = 0U;
             i < initialized_count;
             ++i)
        {
            device_gui_request_shutdown(
                &gui_states[i]
            );
        }
    }

#endif


    if (stop_threads != 0)
    {
        /*
         * recv()에서 대기 중인 소켓을 깨운다.
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


#if ENABLE_GUI

    for (size_t i = 0U;
         i < initialized_count;
         ++i)
    {
        device_gui_state_destroy(
            &gui_states[i]
        );
    }

#endif


    sim_logger_shutdown();


    return exit_code;
}