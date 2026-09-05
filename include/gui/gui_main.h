#ifndef GUI_MAIN_H
#define GUI_MAIN_H


#include "gui/device_a_gui_state.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Dear ImGui 메인 루프
 *
 * 반환값:
 *  0: 정상 종료
 * -1: GLFW/OpenGL/ImGui 초기화 실패
 */
int gui_run(
    device_a_gui_state_t *gui_state
);


#ifdef __cplusplus
}
#endif


#endif