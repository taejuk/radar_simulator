#include "config.h"

#include "common/comm_thread.h"

#include "device/device_a.h"
#include "device/device_b.h"
#include "device/device_c.h"
#include "device/device_d.h"
#include "device/device_e.h"
#include "device/device_f.h"

#include <pthread.h>
#include <stdio.h>


int main(void)
{
    pthread_t threads[DEVICE_COUNT];


    device_context_t devices[DEVICE_COUNT] =
    {
        {
            .device_id = 1,
            .port = DEVICE_A_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_a_handler
        },

        {
            .device_id = 2,
            .port = DEVICE_B_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_b_handler
        },

        {
            .device_id = 3,
            .port = DEVICE_C_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_c_handler
        },

        {
            .device_id = 4,
            .port = DEVICE_D_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_d_handler
        },

        {
            .device_id = 5,
            .port = DEVICE_E_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_e_handler
        },

        {
            .device_id = 6,
            .port = DEVICE_F_PORT,
            .listen_fd = -1,
            .client_fd = -1,
            .is_server = 0,
            .state = 0,
            .packet_handler = device_f_handler
        }
    };


    for (int i = 0; i < DEVICE_COUNT; ++i)
    {
        int ret;

        ret = pthread_create(
            &threads[i],
            NULL,
            comm_thread,
            &devices[i]
        );

        if (ret != 0)
        {
            fprintf(
                stderr,
                "pthread_create failed: device=%d\n",
                devices[i].device_id
            );

            return 1;
        }
    }


    for (int i = 0; i < DEVICE_COUNT; ++i)
    {
        pthread_join(
            threads[i],
            NULL
        );
    }


    return 0;
}