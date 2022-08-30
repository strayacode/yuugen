#include "Common/format.h"
#include "Frontend/HostInterface.h"

void HostInterface::render_watchpoints_window(Arch arch) {
    std::string name = format("%s Watchpoints", arch == Arch::ARMv5 ? "ARM9" : "ARM7");
    CPUBase& cpu = m_system.cpu(static_cast<int>(arch));

    ImGui::Begin(name.c_str());

    static u32 watchpoint_addr = 0;
    ImGui::InputScalar("Watchpoint Address", ImGuiDataType_U32, &watchpoint_addr, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);

    if (ImGui::Button("Add")) {
        cpu.m_watchpoints.add(watchpoint_addr);
    }

    for (auto& watchpoint : cpu.m_watchpoints.get()) {
        ImGui::Text("%08x", watchpoint.addr);
    }

    ImGui::End();
}