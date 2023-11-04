#include "common/logger.h"
#include "nds/system.h"

void compare_states(arm::Arch arch, arm::CPU& a, arm::CPU& b) {
    for (int i = 0; i < 16; i++) {
        if (a.get_gpr(static_cast<arm::GPR>(i)) != b.get_gpr(static_cast<arm::GPR>(i))) {
            logger.error("%s r%d mismatch at %08x: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", i, a.get_gpr(arm::GPR::PC), a.get_gpr(static_cast<arm::GPR>(i)), b.get_gpr(static_cast<arm::GPR>(i)));
        }
    }

    if (a.get_cpsr().data != b.get_cpsr().data) {
        logger.error("%s cpsr mismatch at %08x: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", b.get_gpr(arm::GPR::PC),  a.get_cpsr().data, b.get_cpsr().data);
    }
}

void run_and_compare_cpus(arm::CPU& a, arm::CPU& b, int cycles) {
    for (int i = 0; i < cycles; i++) {
        a.run(1);
        b.run(1);
        compare_states(a.get_arch(), a, b);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        logger.todo("yuugen_tests: no game path was supplied");
        return 1;
    }

    nds::System a_system;
    nds::System b_system;

    a_system.set_update_callback([](f32) {});
    b_system.set_update_callback([](f32) {});

    a_system.select_cpu_backend(arm::BackendType::Interpreter, false);
    b_system.select_cpu_backend(arm::BackendType::IRInterpreter, false);

    a_system.set_game_path(argv[1]);
    a_system.set_boot_mode(common::BootMode::Fast);
    a_system.reset();

    b_system.set_game_path(argv[1]);
    b_system.set_boot_mode(common::BootMode::Fast);
    b_system.reset();

    auto& a_arm7 = a_system.arm7;
    auto& a_arm9 = a_system.arm9;
    auto& b_arm7 = b_system.arm7;
    auto& b_arm9 = b_system.arm9;

    while (true) {
        run_and_compare_cpus(a_arm9.get_cpu(), b_arm9.get_cpu(), 32);
        run_and_compare_cpus(a_arm7.get_cpu(), b_arm7.get_cpu(), 16);

        a_system.scheduler.tick(16);
        a_system.scheduler.run();

        b_system.scheduler.tick(16);
        b_system.scheduler.run();
    }

    return 0;
}