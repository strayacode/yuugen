#include "imgui/imgui.h"
#include "Common/Types.h"
#include "Common/format.h"
#include "Frontend/OnScreenDisplay.h"

OnScreenDisplay::OnScreenDisplay() {
    messages.clear();
}

void OnScreenDisplay::render_messages() {
    auto now = std::chrono::system_clock::now();
    u64 i = 0;
    int x = 0;
    int y = 0;
    
    while (i < messages.size()) {
        auto time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(now - messages[i].timestamp).count();

        if (time_passed >= messages[i].display_duration + messages[i].fade_duration) {
            messages.erase(messages.begin() + i);
        } else {
            render_message(i, messages[i], padding_left + x, padding_top + y, time_passed);
            y += 35;
            i++;
        }
    }
}

void OnScreenDisplay::add_message(std::string text, int display_duration) {
    Message message;
    message.text = text;
    message.timestamp = std::chrono::system_clock::now();
    message.display_duration = display_duration;
    messages.push_back(message);
}

void OnScreenDisplay::render_message(int index, Message& message, int x, int y, int time_passed) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;

    float time_since_display = time_passed - message.display_duration;
    float alpha = time_since_display >= 0 ? 1.0f - (time_since_display / static_cast<float>(message.fade_duration)) : 1.0f;

    std::string title = format("osd %d", index);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::Begin(title.c_str(), nullptr, flags);
    ImGui::Text("%s", message.text.c_str());
    ImGui::End();

    ImGui::PopStyleVar();
}