static int response_type =
    PACKET_STATUS;

static int response_value = 1;
static int response_mode = 1;
static int response_status = 0;

static char response_payload[
    MAX_PAYLOAD_SIZE
] = "DEVICE A STARTED";


packet_header_t request_header = {};

uint8_t request_payload[
    MAX_PAYLOAD_SIZE + 1U
] = {};


int has_request =
    device_a_gui_get_request(
        gui_state,
        &request_header,
        request_payload
    );


ImGui::Begin("Device A Packet");


if (has_request == 0)
{
    ImGui::Text(
        "Waiting for a packet..."
    );
}
else
{
    ImGui::Text(
        "Received request"
    );

    ImGui::Separator();

    ImGui::Text(
        "type: %u",
        (unsigned int)request_header.type
    );

    ImGui::Text(
        "length: %u",
        (unsigned int)request_header.length
    );

    ImGui::Text(
        "seq: %u",
        (unsigned int)request_header.seq
    );

    ImGui::Text(
        "payload: %s",
        (const char *)request_payload
    );


    ImGui::SeparatorText(
        "Response"
    );


    ImGui::InputInt(
        "type",
        &response_type
    );

    ImGui::InputInt(
        "value",
        &response_value
    );

    ImGui::InputInt(
        "mode",
        &response_mode
    );

    ImGui::InputInt(
        "status",
        &response_status
    );

    ImGui::InputTextMultiline(
        "payload",
        response_payload,
        sizeof(response_payload)
    );


    if (ImGui::Button("Send"))
    {
        if ((response_type >= 0) &&
            (response_type <= UINT16_MAX) &&
            (response_value >= 0) &&
            (response_mode >= 0) &&
            (response_mode <= UINT8_MAX) &&
            (response_status >= 0) &&
            (response_status <= UINT8_MAX))
        {
            device_a_gui_submit_response(
                gui_state,
                (uint16_t)response_type,
                (uint32_t)response_value,
                (uint8_t)response_mode,
                (uint8_t)response_status,
                response_payload
            );
        }
    }
}


ImGui::End();