#include "imgui/imgui.h"
#include "common/string.h"
#include "frontend/arm_debugger_window.h"
#include "core/system.h"

ARMDebuggerWindow::ARMDebuggerWindow(core::System& system, arm::CPU& cpu, core::IRQ& irq, arm::Arch arch, FontDatabase& font_database) : system(system), cpu(cpu), irq(irq), arch(arch), font_database(font_database) {}

void ARMDebuggerWindow::render() {
    if (!visible) return;

    ImGui::Begin(common::format("ARM%s Debugger", arch == arm::Arch::ARMv5 ? "9" : "7").c_str());

    font_database.push_style(FontDatabase::Style::Debug);

    auto& state = cpu.get_state();
    ImGui::Text("Registers");
    ImGui::Text("r0: %08x r8:  %08x", state.gpr[0], state.gpr[8]);
    ImGui::Text("r1: %08x r9:  %08x", state.gpr[1], state.gpr[9]);
    ImGui::Text("r2: %08x r10: %08x", state.gpr[2], state.gpr[10]);
    ImGui::Text("r3: %08x r11: %08x", state.gpr[3], state.gpr[11]);
    ImGui::Text("r4: %08x r12: %08x", state.gpr[4], state.gpr[12]);
    ImGui::Text("r5: %08x sp:  %08x", state.gpr[5], state.gpr[13]);
    ImGui::Text("r6: %08x lr:  %08x", state.gpr[6], state.gpr[14]);
    ImGui::Text("r7: %08x pc:  %08x", state.gpr[7], state.gpr[15]);

    auto n = state.cpsr.n;
    auto z = state.cpsr.z;
    auto c = state.cpsr.c;
    auto v = state.cpsr.v;
    auto q = state.cpsr.q;
    auto i = state.cpsr.i;
    auto f = state.cpsr.f;
    auto t = state.cpsr.t;

    ImGui::Text("cpsr: %08x", state.cpsr.data);
    ImGui::Checkbox("N", &n);
    ImGui::SameLine();
    ImGui::Checkbox("Z", &z);
    ImGui::SameLine();
    ImGui::Checkbox("C", &c);
    ImGui::SameLine();
    ImGui::Checkbox("V", &v);
    
    ImGui::Checkbox("Q", &q);
    ImGui::SameLine();
    ImGui::Checkbox("I", &i);
    ImGui::SameLine();
    ImGui::Checkbox("F", &f);
    ImGui::SameLine();
    ImGui::Checkbox("T", &t);

    if (state.spsr) {
        auto n = state.spsr->n;
        auto z = state.spsr->z;
        auto c = state.spsr->c;
        auto v = state.spsr->v;
        auto q = state.spsr->q;
        auto i = state.spsr->i;
        auto f = state.spsr->f;
        auto t = state.spsr->t;

        ImGui::Text("spsr: %08x", state.spsr->data);
        ImGui::Checkbox("N", &n);
        ImGui::SameLine();
        ImGui::Checkbox("Z", &z);
        ImGui::SameLine();
        ImGui::Checkbox("C", &c);
        ImGui::SameLine();
        ImGui::Checkbox("V", &v);
        
        ImGui::Checkbox("Q", &q);
        ImGui::SameLine();
        ImGui::Checkbox("I", &i);
        ImGui::SameLine();
        ImGui::Checkbox("F", &f);
        ImGui::SameLine();
        ImGui::Checkbox("T", &t);
    } else {
        ImGui::Text("spsr: ...");
    }
    
    auto halted = cpu.is_halted();
    ImGui::Checkbox("Halted", &halted);

    ImGui::Separator();

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

    ImGui::Separator();
    ImGui::Text("ime: %08x", irq.read_ime());
    ImGui::SameLine();
    ImGui::Text("ie: %08x", irq.read_ie());
    ImGui::SameLine();
    ImGui::Text("irf: %08x", irq.read_irf());

    font_database.pop_style();
    ImGui::End();
}