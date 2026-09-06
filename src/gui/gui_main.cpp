#include "gui/gui_main.h"

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


static void draw_device_a_window(
    device_a_gui_state_t *gui_state)
{

    /*
     * 하나의 요청에 Send 버튼을 여러 번 누르는 것을 방지한다.
     */
    static device_a_response_packet_t response_packet =
    {
        static_cast<std::uint16_t>(
            PACKET_STATUS
        ),
        0U, /* time_sec */
        0U, /* time_nsec */
        0U, /* radar_status */
        1U, /* mode */
        0U  /* status */
    };
    static bool response_submitted =
        false;

    static bool request_observed =
        false;

    static std::uint32_t observed_seq =
        0U;


    packet_header_t request_header = {};

    std::uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ] = {};


    int has_request =
        device_a_gui_get_request(
            gui_state,
            &request_header,
            request_payload
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
         * 이전 요청이 완료되었으므로
         * 다음 요청에서 다시 Send할 수 있도록 초기화한다.
         */
        response_submitted = false;
        request_observed = false;


        ImGui::Text(
            "Waiting for a packet..."
        );

        ImGui::End();

        return;
    }


    /*
     * 새로운 요청을 확인한 경우
     * Send 상태를 초기화한다.
     */
    if ((!request_observed) ||
        (observed_seq != request_header.seq))
    {
        request_observed = true;

        observed_seq =
            request_header.seq;

        response_submitted = false;
    }


    /*
     * =====================================
     * 받은 요청 표시
     * =====================================
     */
    ImGui::Text(
        "Received request"
    );

    ImGui::Separator();


    ImGui::Text(
        "type: %u",
        static_cast<unsigned int>(
            request_header.type
        )
    );

    ImGui::Text(
        "length: %u",
        static_cast<unsigned int>(
            request_header.length
        )
    );

    ImGui::Text(
        "seq: %u",
        static_cast<unsigned int>(
            request_header.seq
        )
    );

    ImGui::Text(
        "value: %u",
        static_cast<unsigned int>(
            request_header.value
        )
    );

    ImGui::Text(
        "mode: %u",
        static_cast<unsigned int>(
            request_header.mode
        )
    );

    ImGui::Text(
        "status: %u",
        static_cast<unsigned int>(
            request_header.status
        )
    );


    ImGui::Text(
        "payload:"
    );

    ImGui::SameLine();

    ImGui::TextUnformatted(
        reinterpret_cast<const char *>(
            request_payload
        )
    );

    ImGui::SeparatorText(
    "Response");


    ImGui::BeginDisabled(
        response_submitted
    );


    ImGui::InputScalar(
        "message_type",
        ImGuiDataType_U16,
        &response_packet.message_type
    );

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
    * 나노초는 0~999,999,999 범위로 제한한다.
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


    if (ImGui::Button(
            "Send"))
    {
        const int ret =
            device_a_gui_submit_response(
                gui_state,
                &response_packet
            );


        if (ret == 0)
        {
            response_submitted = true;
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
            "Response submitted for seq=%u",
            static_cast<unsigned int>(
                request_header.seq
            )
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


    if (glfwInit() == GLFW_FALSE)
    {
        std::fprintf(
            stderr,
            "glfwInit failed\n"
        );

        return -1;
    }


    glfwDefaultWindowHints();
    /*
     * GLFW Window와 OpenGL Context 생성
     */
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
    glfwSwapInterval(1);


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
         * 불필요하게 계속 렌더링하지 않는다.
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


        /*
         * Device A 패킷 입력 화면
         */
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