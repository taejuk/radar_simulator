#include "common/comm_thread.h"

#include "device/device_a.h"
#include "device/device_b.h"
#include "device/device_c.h"
#include "device/device_d.h"
#include "device/device_e.h"
#include "device/device_f.h"

#include <pthread.h>
#include <stdio.h>


#define DEVICE_COUNT 6


int main(void)
{
    pthread_t threads[
        DEVICE_COUNT
    ];


    device_context_t devices[
        DEVICE_COUNT
    ] =
    {
        {
            .device_id = 1,

            .port = 5001,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_a_handler
        },


        {
            .device_id = 2,

            .port = 5002,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_b_handler
        },


        {
            .device_id = 3,

            .port = 5003,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_c_handler
        },


        {
            .device_id = 4,

            .port = 5004,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_d_handler
        },


        {
            .device_id = 5,

            .port = 5005,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_e_handler
        },


        {
            .device_id = 6,

            .port = 5006,

            .listen_fd = -1,

            .client_fd = -1,

            .state = 0,

            .packet_handler =
                device_f_handler
        }
    };


    /*
     * ==============================
     * Thread 생성
     * ==============================
     */

    for (int i = 0;
         i < DEVICE_COUNT;
         ++i)
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
                "pthread_create failed "
                "for device %d\n",
                devices[i].device_id
            );


            return 1;
        }
    }


    /*
     * ==============================
     * Thread 종료 대기
     * ==============================
     */

    for (int i = 0;
         i < DEVICE_COUNT;
         ++i)
    {
        pthread_join(
            threads[i],
            NULL
        );
    }


    return 0;
}