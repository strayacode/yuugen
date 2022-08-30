#include "Common/format.h"
#include "Frontend/HostInterface.h"

void HostInterface::render_disassembly_window(Arch arch) {
    std::string name = format("%s Disassembly", arch == Arch::ARMv5 ? "ARM9" : "ARM7");
    CPUBase& cpu = m_system.cpu(static_cast<int>(arch));

    ImGui::Begin(name.c_str());

    if (ImGui::Button("+")) {
        disassembly_size++;
    }

    ImGui::SameLine();

    if (ImGui::Button("-")) {
        disassembly_size--;
    }

    ImGui::SameLine();

    if (disassembly_size < 0) {
        disassembly_size = 0;
    }

    ImGui::Text("Number of Instructions: %d", disassembly_size);

    if (m_system.state() != State::Idle) {
        int increment = cpu.is_arm() ? 4 : 2;
        u32 pc = cpu.m_gpr[15];
        u32 addr = pc - ((disassembly_size - 1) / 2) * increment;
        
        if (cpu.is_arm()) {
            for (int i = 0; i < disassembly_size; i++) {
                u32 instruction = arch == Arch::ARMv5 ? m_system.arm9.memory().read<u32>(addr) : m_system.arm7.memory().read<u32>(addr);
                if (addr == pc) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X:", addr);
                    ImGui::SameLine(67);
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X", instruction);
                    ImGui::SameLine(125);
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", disassembler.disassemble_arm(instruction).c_str());
                } else {
                    ImGui::Text("%08X:", addr);
                    ImGui::SameLine(67);
                    ImGui::Text("%08X", instruction);
                    ImGui::SameLine(125);
                    ImGui::Text("%s", disassembler.disassemble_arm(instruction).c_str());
                }
                
                addr += increment;
            }
        } else {
            for (int i = 0; i < disassembly_size; i++) {
                u32 instruction = arch == Arch::ARMv5 ? m_system.arm9.memory().read<u16>(addr) : m_system.arm7.memory().read<u16>(addr);
                if (addr == pc) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X:", addr);
                    ImGui::SameLine(67);
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08X", instruction);
                    ImGui::SameLine(125);
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", disassembler.disassemble_thumb(instruction).c_str());
                } else {
                    ImGui::Text("%08X:", addr);
                    ImGui::SameLine(67);
                    ImGui::Text("%08X", instruction);
                    ImGui::SameLine(125);
                    ImGui::Text("%s", disassembler.disassemble_thumb(instruction).c_str());
                }

                addr += increment;
            }
        }
    }

    ImGui::End();
}