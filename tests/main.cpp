#include "common/logger.h"
#include "core/system.h"

void compare_states(arm::Arch arch, arm::CPU& a, arm::CPU& b) {
    for (int i = 0; i < 16; i++) {
        if (a.get_gpr(static_cast<arm::GPR>(i)) != b.get_gpr(static_cast<arm::GPR>(i))) {
            logger.error("%s r%d mismatch: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", i, a.get_gpr(static_cast<arm::GPR>(i)), b.get_gpr(static_cast<arm::GPR>(i)));
        }
    }

    if (a.get_cpsr().data != b.get_cpsr().data) {
        logger.error("%s cpsr mismatch: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", a.get_cpsr().data, b.get_cpsr().data);
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

    core::System interpreter_system;
    core::System ir_interpreter_system;

    interpreter_system.set_update_callback([](f32) {});
    ir_interpreter_system.set_update_callback([](f32) {});

    interpreter_system.select_cpu_backend(arm::BackendType::Interpreter);
    ir_interpreter_system.select_cpu_backend(arm::BackendType::IRInterpreter);

    interpreter_system.set_game_path(argv[1]);
    interpreter_system.set_boot_mode(core::BootMode::Direct);
    interpreter_system.reset();

    ir_interpreter_system.set_game_path(argv[1]);
    ir_interpreter_system.set_boot_mode(core::BootMode::Direct);
    ir_interpreter_system.reset();

    logger.debug("running interpreter and jit...");

    auto& interpreter_arm7 = interpreter_system.arm7;
    auto& interpreter_arm9 = interpreter_system.arm9;
    auto& ir_interpreter_arm7 = ir_interpreter_system.arm7;
    auto& ir_interpreter_arm9 = ir_interpreter_system.arm9;

    while (true) {
        run_and_compare_cpus(interpreter_arm9.get_cpu(), ir_interpreter_arm9.get_cpu(), 2);
        run_and_compare_cpus(interpreter_arm7.get_cpu(), ir_interpreter_arm7.get_cpu(), 1);

        interpreter_system.scheduler.tick(1);
        interpreter_system.scheduler.run();

        ir_interpreter_system.scheduler.tick(1);
        ir_interpreter_system.scheduler.run();
    }

    return 0;
}