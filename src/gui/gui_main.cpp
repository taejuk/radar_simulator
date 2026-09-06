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

    packet.mode = 1U;
    packet.radar_status = 1U;
    packet.status = 1U;
}

/*
 * 체크박스와 숫자 값의 관계
 *
 * 체크됨     -> 0
 * 체크 안 됨 -> 1
 */
template <typename IntegerType>
static void draw_zero_when_checked(
    const char *label,
    IntegerType &value)
{
    /*
     * 0/1 범위를 벗어난 값은 1로 제한한다.
     */
    if ((value !=
         static_cast<IntegerType>(0)) &&
        (value !=
         static_cast<IntegerType>(1)))
    {
        value =
            static_cast<IntegerType>(1);
    }


    /*
     * ImGui::Checkbox()에는 bool 변수를 전달해야 한다.
     *
     * uint8_t* 또는 uint32_t*를 bool*로
     * 직접 형 변환하면 안 된다.
     */
    bool checked =
        (value ==
         static_cast<IntegerType>(0));


    if (ImGui::Checkbox(
            label,
            &checked))
    {
        value =
            checked ?
                static_cast<IntegerType>(0) :
                static_cast<IntegerType>(1);
    }


    /*
     * 실제 전송될 숫자를 옆에 표시한다.
     */
    ImGui::SameLine();


    ImGui::TextDisabled(
        "send value: %u",
        static_cast<unsigned int>(
            value
        )
    );
}

/*
 * BIT 테이블의 다음 칸에 체크박스를 그린다.
 *
 * column_count를 초과하면 ImGui가
 * 자동으로 다음 행으로 이동한다.
 */
template <typename IntegerType>
static void draw_next_bit_item(
    const char *label,
    IntegerType &value)
{
    ImGui::TableNextColumn();


    draw_zero_when_checked(
        label,
        value
    );
}


/*
 * =====================================
 * Device A 전용 입력 화면
 * =====================================
 *
 * radar_status, mode, status만
 * 0/1 체크박스로 표시한다.
 */
static void draw_response_fields(
    device_a_response_packet_t &packet)
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


    /*
     * 나노초 범위 제한
     */
    if (packet.time_nsec >
        999999999U)
    {
        packet.time_nsec =
            999999999U;
    }


    ImGui::SeparatorText(
        "Built-In Test"
    );


    /*
    * 한 행에 표시할 BIT 항목 수
    *
    * 2: 항목 이름이 긴 경우
    * 3: 일반적인 경우
    * 4: 항목 이름이 짧은 경우
    */
    const int bit_column_count =
        3;


    if (ImGui::BeginTable(
            "DeviceABitTable",
            bit_column_count,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingStretchSame))
    {
        draw_next_bit_item(
            "radar_status",
            packet.radar_status
        );


        draw_next_bit_item(
            "mode",
            packet.mode
        );


        draw_next_bit_item(
            "status",
            packet.status
        );


        /*
        * 필드가 추가되면 아래처럼 계속 작성한다.
        *
        * 세 번째 열까지 채워지면
        * 다음 항목부터 자동으로 다음 행에 표시된다.
        */
        /*
        draw_next_bit_item(
            "power_supply",
            packet.power_supply
        );


        draw_next_bit_item(
            "transmitter",
            packet.transmitter
        );


        draw_next_bit_item(
            "receiver",
            packet.receiver
        );
        */


        ImGui::EndTable();
    }
}


/*
 * =====================================
 * Device B~F 공통 입력 화면
 * =====================================
 *
 * B~F는 기존처럼 숫자 입력 상자를 사용한다.
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