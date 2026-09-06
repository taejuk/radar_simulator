#include "gui/gui_main.h"

#include "common/packet.h"
#include "config.h"

#include "device/device_a_packet.h"
#include "device/device_b_packet.h"
#include "device/device_c_packet.h"
#include "device/device_d_packet.h"
#include "device/device_e_packet.h"
#include "device/device_f_packet.h"

#include "gui/device_gui_state.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>


#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>


typedef struct device_panel_state
{
    bool response_submitted;

    bool response_initialized;

    std::uint64_t observed_generation;

} device_panel_state_t;


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
 * 현재 A~F payload가 동일한 필드를 가지므로
 * 공통 초기화 함수를 사용한다.
 */
template <typename PacketType>
static void initialize_response_packet(
    PacketType &packet)
{
    packet = {};

    packet.message_type =
        static_cast<std::uint16_t>(
            PACKET_STATUS
        );

    packet.mode =
        1U;
}


/*
 * 현재 A~F payload에 공통으로 존재하는 필드 편집기다.
 *
 * 나중에 특정 Device payload 필드가 달라지면
 * 해당 Device 전용 draw 함수를 만들어 호출하면 된다.
 */
template <typename PacketType>
static void draw_response_fields(
    PacketType &packet)
{
    ImGui::InputScalar(
        "message_type",
        ImGuiDataType_U16,
        &packet.message_type
    );


    ImGui::InputScalar(
        "time_sec",
        ImGuiDataType_U32,
        &packet.time_sec
    );


    ImGui::InputScalar(
        "time_nsec",
        ImGuiDataType_U32,
        &packet.time_nsec
    );


    if (packet.time_nsec >
        999999999U)
    {
        packet.time_nsec =
            999999999U;
    }


    ImGui::InputScalar(
        "radar_status",
        ImGuiDataType_U32,
        &packet.radar_status
    );


    ImGui::InputScalar(
        "mode",
        ImGuiDataType_U8,
        &packet.mode
    );


    ImGui::InputScalar(
        "status",
        ImGuiDataType_U8,
        &packet.status
    );
}


static void draw_received_header(
    const InternalMsgHeader_t &header)
{
    ImGui::Text(
        "msgType: %u",
        static_cast<unsigned int>(
            header.msgType
        )
    );


    ImGui::Text(
        "msgSize: %u",
        static_cast<unsigned int>(
            header.msgSize
        )
    );


    ImGui::Text(
        "msgSec: %u",
        static_cast<unsigned int>(
            header.msgSec
        )
    );


    ImGui::Text(
        "msgNSec: %u",
        static_cast<unsigned int>(
            header.msgNSec
        )
    );


    ImGui::Text(
        "srcId: %u",
        static_cast<unsigned int>(
            header.srcId
        )
    );


    ImGui::Text(
        "destId: %u",
        static_cast<unsigned int>(
            header.destId
        )
    );
}


static void draw_payload_preview(
    const std::uint8_t *payload,
    std::uint32_t payload_size)
{
    const std::uint32_t preview_size =
        (payload_size < 32U) ?
            payload_size : 32U;


    ImGui::Text(
        "Payload preview (%u bytes):",
        static_cast<unsigned int>(
            payload_size
        )
    );


    if (preview_size == 0U)
    {
        ImGui::TextDisabled(
            "empty"
        );

        return;
    }


    for (std::uint32_t i = 0U;
         i < preview_size;
         ++i)
    {
        if (i != 0U)
        {
            ImGui::SameLine(
                0.0F,
                4.0F
            );
        }


        ImGui::Text(
            "%02X",
            static_cast<unsigned int>(
                payload[i]
            )
        );
    }


    if (payload_size >
        preview_size)
    {
        ImGui::TextDisabled(
            "... first 32 bytes shown"
        );
    }
}


template <typename PacketType>
static void draw_device_panel(
    const char *device_name,
    device_gui_state_t *gui_state,
    PacketType &response_packet,
    device_panel_state_t &panel_state)
{
    InternalMsgHeader_t
        request_header = {};


    std::uint8_t request_payload[
        MAX_PAYLOAD_SIZE + 1U
    ] = {};


    std::uint64_t request_generation =
        0U;


    const int has_request =
        device_gui_get_request(
            gui_state,
            &request_header,
            request_payload,
            &request_generation
        );


    if (!panel_state.response_initialized)
    {
        initialize_response_packet(
            response_packet
        );

        panel_state.response_initialized =
            true;
    }


    ImGui::Text(
        "%s",
        device_name
    );

    ImGui::Separator();


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

        return;
    }


    if (has_request == 0)
    {
        panel_state.response_submitted =
            false;

        ImGui::Text(
            "Waiting for a packet..."
        );

        return;
    }


    /*
     * 동일한 Header가 다시 들어온 경우에도
     * generation 값으로 새 요청을 구분한다.
     */
    if (panel_state.observed_generation !=
        request_generation)
    {
        panel_state.observed_generation =
            request_generation;

        panel_state.response_submitted =
            false;
    }


    ImGui::SeparatorText(
        "Received Header"
    );


    draw_received_header(
        request_header
    );


    draw_payload_preview(
        request_payload,
        request_header.msgSize
    );


    ImGui::SeparatorText(
        "Response Payload"
    );


    ImGui::BeginDisabled(
        panel_state.response_submitted
    );


    ImGui::PushItemWidth(
        250.0F
    );


    draw_response_fields(
        response_packet
    );


    ImGui::PopItemWidth();


    if (ImGui::Button(
            "Send"))
    {
        const int submit_result =
            device_gui_submit_response(
                gui_state,
                response_packet.message_type,
                &response_packet,
                static_cast<std::uint32_t>(
                    sizeof(response_packet)
                )
            );


        if (submit_result == 0)
        {
            panel_state.response_submitted =
                true;
        }
    }


    ImGui::EndDisabled();


    if (panel_state.response_submitted)
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
}


static void draw_device_tabs(
    device_gui_state_t *gui_states,
    std::size_t device_count)
{
    static device_panel_state_t
        panel_states[DEVICE_COUNT] =
        {};


    static device_a_response_packet_t
        device_a_response =
        {};

    static device_b_response_packet_t
        device_b_response =
        {};

    static device_c_response_packet_t
        device_c_response =
        {};

    static device_d_response_packet_t
        device_d_response =
        {};

    static device_e_response_packet_t
        device_e_response =
        {};

    static device_f_response_packet_t
        device_f_response =
        {};


    ImGui::SetNextWindowSize(
        ImVec2(
            620.0F,
            620.0F
        ),
        ImGuiCond_FirstUseEver
    );


    ImGui::Begin(
        "Radar Simulator"
    );


    if (ImGui::BeginTabBar(
            "DeviceTabs"))
    {
        if ((device_count > 0U) &&
            ImGui::BeginTabItem(
                "Device A"))
        {
            ImGui::PushID(0);

            draw_device_panel(
                "Device A",
                &gui_states[0],
                device_a_response,
                panel_states[0]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        if ((device_count > 1U) &&
            ImGui::BeginTabItem(
                "Device B"))
        {
            ImGui::PushID(1);

            draw_device_panel(
                "Device B",
                &gui_states[1],
                device_b_response,
                panel_states[1]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        if ((device_count > 2U) &&
            ImGui::BeginTabItem(
                "Device C"))
        {
            ImGui::PushID(2);

            draw_device_panel(
                "Device C",
                &gui_states[2],
                device_c_response,
                panel_states[2]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        if ((device_count > 3U) &&
            ImGui::BeginTabItem(
                "Device D"))
        {
            ImGui::PushID(3);

            draw_device_panel(
                "Device D",
                &gui_states[3],
                device_d_response,
                panel_states[3]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        if ((device_count > 4U) &&
            ImGui::BeginTabItem(
                "Device E"))
        {
            ImGui::PushID(4);

            draw_device_panel(
                "Device E",
                &gui_states[4],
                device_e_response,
                panel_states[4]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        if ((device_count > 5U) &&
            ImGui::BeginTabItem(
                "Device F"))
        {
            ImGui::PushID(5);

            draw_device_panel(
                "Device F",
                &gui_states[5],
                device_f_response,
                panel_states[5]
            );

            ImGui::PopID();

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }


    ImGui::End();
}


extern "C" int gui_run(
    device_gui_state_t *gui_states,
    std::size_t device_count)
{
    GLFWwindow *window;


    if ((gui_states == nullptr) ||
        (device_count == 0U) ||
        (device_count > DEVICE_COUNT))
    {
        std::fprintf(
            stderr,
            "gui_run: invalid device state array\n"
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


    glfwDefaultWindowHints();


    window = glfwCreateWindow(
        1000,
        700,
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


    glfwSwapInterval(
        1
    );


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


    while (glfwWindowShouldClose(
               window) == GLFW_FALSE)
    {
        int display_width;

        int display_height;


        glfwPollEvents();


        if (glfwGetWindowAttrib(
                window,
                GLFW_ICONIFIED) != 0)
        {
            glfwWaitEventsTimeout(
                0.01
            );

            continue;
        }


        ImGui_ImplOpenGL2_NewFrame();

        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();


        draw_device_tabs(
            gui_states,
            device_count
        );


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


    ImGui_ImplOpenGL2_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();


    glfwDestroyWindow(
        window
    );

    glfwTerminate();


    return 0;
}