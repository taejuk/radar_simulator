#include "gui/gui_main.h"

#include "common/packet.h"
#include "config.h"
#include "gui/device_a_gui_state.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <cstdint>
#include <cstdio>


#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>


static void glfw_error_callback(
    int error,
    const char *description)
{
    std::fprintf(
        stderr,
        "GLFW Error %d: %s\n",
        error,
        description
    );
}


/*
 * 두 요청 Header가 같은지 검사한다.
 *
 * InternalMsgHeader_t에는 seq가 없으므로
 * Header의 모든 필드를 비교한다.
 */
static bool is_same_request_header(
    const InternalMsgHeader_t &left,
    const InternalMsgHeader_t &right)
{
    return
        (left.msgType ==
         right.msgType) &&

        (left.msgSize ==
         right.msgSize) &&

        (left.msgSec ==
         right.msgSec) &&

        (left.msgNSec ==
         right.msgNSec) &&

        (left.srcId ==
         right.srcId) &&

        (left.destId ==
         right.destId);
}


static void draw_device_a_window(
    device_a_gui_state_t *gui_state)
{


    static device_a_response_packet_t
    response_packet = {};

    static bool response_packet_initialized =
        false;


    if (!response_packet_initialized)
    {
        response_packet.message_type =
            static_cast<std::uint16_t>(
                PACKET_STATUS
            );

        response_packet.mode =
            1U;

        /*
        * 새 필드의 기본값
        */
        response_packet.temperature_x10 =
            250; /* 25.0도 */

        response_packet.voltage_mv =
            24000U;

        response_packet.fault_code =
            0U;


        response_packet_initialized =
            true;
    }

    /*
     * 하나의 요청에 Send 버튼을 여러 번
     * 누르는 것을 방지한다.
     */
    static bool response_submitted =
        false;


    /*
     * 새로운 요청인지 확인하기 위한 상태
     */
    static bool request_observed =
        false;

    static InternalMsgHeader_t
        observed_request_header =
        {};


    /*
     * GUI 공유 상태에서 복사할 요청
     */
    InternalMsgHeader_t
        request_header =
        {};

    std::uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ] =
    {
        0
    };


    const int has_request =
        device_a_gui_get_request(
            gui_state,
            &request_header,
            request_payload
        );


    /*
     * 최초 실행 시 Device A 창 크기 설정
     */
    ImGui::SetNextWindowSize(
        ImVec2(
            520.0F,
            560.0F
        ),
        ImGuiCond_FirstUseEver
    );


    ImGui::Begin(
        "Device A Packet"
    );


    if (has_request < 0)
    {
        ImGui::TextColored(
            ImVec4(
                1.0F,
                0.2F,
                0.2F,
                1.0F
            ),
            "Failed to read GUI shared state"
        );


        ImGui::End();

        return;
    }


    if (has_request == 0)
    {
        /*
         * 처리 중인 요청이 없으므로
         * 다음 요청에서 다시 Send할 수 있도록 한다.
         */
        response_submitted =
            false;

        request_observed =
            false;


        ImGui::Text(
            "Waiting for a packet..."
        );


        ImGui::End();

        return;
    }


    /*
     * 새로운 요청이면 Send 상태를 초기화한다.
     *
     * 기존 Header의 seq 필드가 없어졌으므로
     * InternalMsgHeader_t 전체 필드로 비교한다.
     */
    if ((!request_observed) ||
        (!is_same_request_header(
            observed_request_header,
            request_header)))
    {
        request_observed =
            true;

        observed_request_header =
            request_header;

        response_submitted =
            false;
    }


    /*
     * =====================================
     * 수신 Header 표시
     * =====================================
     *
     * request_header는 comm_thread에서 이미
     * Big Endian에서 Host Endian으로 변환된 상태이다.
     */
    ImGui::Text(
        "Received InternalMsgHeader"
    );

    ImGui::Separator();


    ImGui::Text(
        "msgType: %u",
        static_cast<unsigned int>(
            request_header.msgType
        )
    );

    ImGui::Text(
        "msgSize: %u",
        static_cast<unsigned int>(
            request_header.msgSize
        )
    );

    ImGui::Text(
        "msgSec: %u",
        static_cast<unsigned int>(
            request_header.msgSec
        )
    );

    ImGui::Text(
        "msgNSec: %u",
        static_cast<unsigned int>(
            request_header.msgNSec
        )
    );

    ImGui::Text(
        "srcId: %u",
        static_cast<unsigned int>(
            request_header.srcId
        )
    );

    ImGui::Text(
        "destId: %u",
        static_cast<unsigned int>(
            request_header.destId
        )
    );


    /*
     * 현재 테스트 payload는 문자열이므로
     * 문자열 형태로 출력한다.
     *
     * 실제 payload가 바이너리 구조체로 바뀌면
     * 이 출력 부분도 구조체 필드 출력으로 바꿔야 한다.
     */
    ImGui::Text(
        "payload:"
    );

    ImGui::SameLine();

    ImGui::TextUnformatted(
        reinterpret_cast<const char *>(
            request_payload
        )
    );


    /*
     * =====================================
     * 응답 Payload 입력
     * =====================================
     */
    ImGui::SeparatorText(
        "Response Payload"
    );


    /*
     * 이미 응답을 제출했다면
     * 입력 상자와 Send 버튼을 비활성화한다.
     */
    ImGui::BeginDisabled(
        response_submitted
    );


    /*
     * 입력 상자의 너비를 지정한다.
     */
    ImGui::PushItemWidth(
        250.0F
    );


    /*
     * 이 값은 응답 Header의 msgType으로도 사용된다.
     */
    ImGui::InputScalar(
        "message_type",
        ImGuiDataType_U16,
        &response_packet.message_type
    );


    /*
     * 아래 time_sec/time_nsec는
     * device_a_response_packet_t payload의 필드이다.
     *
     * InternalMsgHeader_t의 msgSec/msgNSec은
     * Handler에서 CLOCK_REALTIME으로 따로 설정된다.
     */
    ImGui::InputScalar(
        "time_sec",
        ImGuiDataType_U32,
        &response_packet.time_sec
    );

    ImGui::InputScalar(
        "time_nsec",
        ImGuiDataType_U32,
        &response_packet.time_nsec
    );


    /*
     * payload의 time_nsec 범위 제한
     */
    if (response_packet.time_nsec >
        999999999U)
    {
        response_packet.time_nsec =
            999999999U;
    }


    ImGui::InputScalar(
        "radar_status",
        ImGuiDataType_U32,
        &response_packet.radar_status
    );

    ImGui::InputScalar(
        "mode",
        ImGuiDataType_U8,
        &response_packet.mode
    );

    ImGui::InputScalar(
        "status",
        ImGuiDataType_U8,
        &response_packet.status
    );

    ImGui::InputScalar(
        "temperature_x10",
        ImGuiDataType_S16,
        &response_packet.temperature_x10
    );

    ImGui::InputScalar(
        "voltage_mv",
        ImGuiDataType_U16,
        &response_packet.voltage_mv
    );

    ImGui::InputScalar(
        "fault_code",
        ImGuiDataType_U32,
        &response_packet.fault_code
    );


    ImGui::PopItemWidth();


    if (ImGui::Button(
            "Send"))
    {
        const int submit_result =
            device_a_gui_submit_response(
                gui_state,
                &response_packet
            );


        if (submit_result == 0)
        {
            response_submitted =
                true;
        }
    }


    ImGui::EndDisabled();


    if (response_submitted)
    {
        ImGui::TextColored(
            ImVec4(
                0.2F,
                1.0F,
                0.2F,
                1.0F
            ),
            "Response submitted"
        );
    }


    ImGui::End();
}


extern "C" int gui_run(
    device_a_gui_state_t *gui_state)
{
    GLFWwindow *window;


    if (gui_state == nullptr)
    {
        std::fprintf(
            stderr,
            "gui_run: gui_state is NULL\n"
        );

        return -1;
    }


    glfwSetErrorCallback(
        glfw_error_callback
    );


    if (glfwInit() ==
        GLFW_FALSE)
    {
        std::fprintf(
            stderr,
            "glfwInit failed\n"
        );

        return -1;
    }


    /*
     * 운영체제와 GLFW가 선택한
     * 기본 OpenGL Context를 사용한다.
     *
     * XQuartz의 구형 OpenGL Context와
     * 호환하기 위해 특정 버전을 요청하지 않는다.
     */
    glfwDefaultWindowHints();


    window = glfwCreateWindow(
        900,
        600,
        "Radar Simulator",
        nullptr,
        nullptr
    );


    if (window == nullptr)
    {
        std::fprintf(
            stderr,
            "glfwCreateWindow failed\n"
        );

        glfwTerminate();

        return -1;
    }


    glfwMakeContextCurrent(
        window
    );


    /*
     * VSync 사용
     */
    glfwSwapInterval(
        1
    );


    /*
     * =====================================
     * Dear ImGui 초기화
     * =====================================
     */
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();


    ImGuiIO &io =
        ImGui::GetIO();

    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;


    ImGui::StyleColorsDark();


    if (!ImGui_ImplGlfw_InitForOpenGL(
            window,
            true))
    {
        std::fprintf(
            stderr,
            "ImGui GLFW backend init failed\n"
        );


        ImGui::DestroyContext();

        glfwDestroyWindow(
            window
        );

        glfwTerminate();

        return -1;
    }


    if (!ImGui_ImplOpenGL2_Init())
    {
        std::fprintf(
            stderr,
            "ImGui OpenGL2 backend init failed\n"
        );


        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();

        glfwDestroyWindow(
            window
        );

        glfwTerminate();

        return -1;
    }


    /*
     * =====================================
     * GUI Main Loop
     * =====================================
     */
    while (glfwWindowShouldClose(
               window) == GLFW_FALSE)
    {
        int display_width;
        int display_height;


        glfwPollEvents();


        /*
         * 창이 최소화된 동안에는
         * 불필요한 렌더링을 하지 않는다.
         */
        if (glfwGetWindowAttrib(
                window,
                GLFW_ICONIFIED) != 0)
        {
            glfwWaitEventsTimeout(
                0.01
            );

            continue;
        }


        /*
         * 새로운 ImGui Frame 시작
         */
        ImGui_ImplOpenGL2_NewFrame();

        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();


        draw_device_a_window(
            gui_state
        );


        /*
         * ImGui 렌더링 데이터 생성
         */
        ImGui::Render();


        glfwGetFramebufferSize(
            window,
            &display_width,
            &display_height
        );


        glViewport(
            0,
            0,
            display_width,
            display_height
        );


        glClearColor(
            0.10F,
            0.10F,
            0.12F,
            1.0F
        );


        glClear(
            GL_COLOR_BUFFER_BIT
        );


        ImGui_ImplOpenGL2_RenderDrawData(
            ImGui::GetDrawData()
        );


        glfwSwapBuffers(
            window
        );
    }


    /*
     * =====================================
     * 종료 처리
     * =====================================
     */
    ImGui_ImplOpenGL2_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();


    glfwDestroyWindow(
        window
    );

    glfwTerminate();


    return 0;
}