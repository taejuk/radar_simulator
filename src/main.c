#include "config.h"

#include "common/comm_thread.h"

#include "device/device_a.h"

#include "gui/device_a_gui_state.h"
#include "gui/gui_main.h"

#include <pthread.h>
#include <stdio.h>


int main(void)
{
    pthread_t device_a_thread;

    device_a_gui_state_t device_a_gui_state;

    device_context_t device_a;

    int ret;
    int gui_ret;


    /*
     * =====================================
     * Device A GUI 공유 상태 초기화
     * =====================================
     */
    ret = device_a_gui_state_init(
        &device_a_gui_state
    );

    if (ret < 0)
    {
        fprintf(
            stderr,
            "device_a_gui_state_init failed\n"
        );

        return 1;
    }


    /*
     * =====================================
     * Device A 통신 정보 초기화
     * =====================================
     */
    device_a.device_id = 1;

    device_a.port =
        DEVICE_A_PORT;

    device_a.listen_fd = -1;

    device_a.client_fd = -1;

    /*
     * Device A는 TCP client로 동작
     */
    device_a.is_server = 0;

    device_a.state = 0;

    /*
     * 통신 스레드와 GUI가 공유할 상태
     */
    device_a.gui_state =
        &device_a_gui_state;

    device_a.packet_handler =
        device_a_handler;


    /*
     * =====================================
     * Device A 통신 스레드 시작
     * =====================================
     */
    ret = pthread_create(
        &device_a_thread,
        NULL,
        comm_thread,
        &device_a
    );

    if (ret != 0)
    {
        fprintf(
            stderr,
            "pthread_create failed: device=%d, error=%d\n",
            device_a.device_id,
            ret
        );

        device_a_gui_state_destroy(
            &device_a_gui_state
        );

        return 1;
    }


    /*
     * =====================================
     * ImGui 실행
     * =====================================
     *
     * ImGui/GLFW/OpenGL 처리는 메인 스레드에서 수행한다.
     * gui_run()은 GUI 창이 닫힐 때 반환한다.
     */
    gui_ret = gui_run(
        &device_a_gui_state
    );


    /*
     * GUI 창이 닫혔으므로
     * 대기 중인 Device A handler를 깨운다.
     */
    device_a_gui_request_shutdown(
        &device_a_gui_state
    );


    /*
     * Device A 통신 스레드 종료 대기
     */
    ret = pthread_join(
        device_a_thread,
        NULL
    );

    if (ret != 0)
    {
        fprintf(
            stderr,
            "pthread_join failed: device=%d, error=%d\n",
            device_a.device_id,
            ret
        );
    }


    /*
     * GUI 공유 자원 해제
     */
    device_a_gui_state_destroy(
        &device_a_gui_state
    );


    if ((gui_ret < 0) ||
        (ret != 0))
    {
        return 1;
    }


    return 0;
}