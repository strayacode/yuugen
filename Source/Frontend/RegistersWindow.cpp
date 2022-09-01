#include "Common/format.h"
#include "Frontend/HostInterface.h"

void HostInterface::render_registers_window(Arch arch) {
    std::string name = format("%s Registers", arch == Arch::ARMv5 ? "ARM9" : "ARM7");
    CPUBase& cpu = m_system.cpu(static_cast<int>(arch));

    ImGui::Begin(name.c_str());

    ImGui::PushFont(m_debugger_font);

    for (int i = 0; i < 16; i++) {
        ImGui::Text("%-10s %08x", disassembler.register_name(i).c_str(), cpu.m_gpr[i]);
    }

    ImGui::PopFont();

    if (cpu.m_halted) {
        ImGui::Text("State: Halted");
    } else {
        ImGui::Text("State: Running");
    }

    ImGui::End();
}