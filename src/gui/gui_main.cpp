#include "gui/gui_main.h"

#include "config.h"
#include "gui/device_a_gui_state.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


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
     * GUI에서 입력할 응답 패킷 값
     */
    static std::uint16_t response_type =
        static_cast<std::uint16_t>(
            PACKET_STATUS
        );

    static std::uint32_t response_value =
        1U;

    static std::uint8_t response_mode =
        1U;

    static std::uint8_t response_status =
        0U;


    static char response_payload[
        MAX_PAYLOAD_SIZE
    ] = "DEVICE A STARTED";


    /*
     * 하나의 요청에 Send 버튼을 여러 번 누르는 것을 방지한다.
     */
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


    /*
     * =====================================
     * 보낼 응답 입력
     * =====================================
     */
    ImGui::SeparatorText(
        "Response"
    );


    /*
     * 이미 Send를 누른 경우
     * 입력 필드와 버튼을 비활성화한다.
     */
    ImGui::BeginDisabled(
        response_submitted
    );


    ImGui::InputScalar(
        "type",
        ImGuiDataType_U16,
        &response_type
    );


    ImGui::InputScalar(
        "value",
        ImGuiDataType_U32,
        &response_value
    );


    ImGui::InputScalar(
        "mode",
        ImGuiDataType_U8,
        &response_mode
    );


    ImGui::InputScalar(
        "status",
        ImGuiDataType_U8,
        &response_status
    );


    ImGui::InputTextMultiline(
        "payload",
        response_payload,
        sizeof(response_payload),
        ImVec2(
            -1.0F,
            100.0F
        )
    );


    if (ImGui::Button("Send"))
    {
        int ret;


        ret = device_a_gui_submit_response(
            gui_state,
            response_type,
            response_value,
            response_mode,
            response_status,
            response_payload
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

    const char *glsl_version;


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


#if defined(__APPLE__)

    /*
     * macOS에서는 OpenGL 3.2 Core Profile 사용
     */
    glsl_version =
        "#version 150";

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        2
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    glfwWindowHint(
        GLFW_OPENGL_FORWARD_COMPAT,
        GLFW_TRUE
    );

#else

    /*
     * Linux에서는 OpenGL 3.0 사용
     */
    glsl_version =
        "#version 130";

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        0
    );

#endif


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


    if (!ImGui_ImplOpenGL3_Init(
            glsl_version))
    {
        std::fprintf(
            stderr,
            "ImGui OpenGL3 backend init failed\n"
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
        ImGui_ImplOpenGL3_NewFrame();

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


        ImGui_ImplOpenGL3_RenderDrawData(
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
    ImGui_ImplOpenGL3_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();


    glfwDestroyWindow(
        window
    );

    glfwTerminate();


    return 0;
}