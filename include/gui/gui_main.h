#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include "gui/device_gui_state.h"

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * GUI는 메인 스레드 하나에서 실행한다.
 *
 * gui_states에는 Device A~F의 상태 배열을 전달한다.
 */
int gui_run(
    device_gui_state_t *gui_states,
    size_t device_count
);


#ifdef __cplusplus
}
#endif


#endif