#include "common/logger.h"
#include "common/system.h"
#include "gba/system.h"
#include "nds/system.h"

void compare_states(arm::Arch arch, arm::CPU& a, arm::CPU& b) {
    for (int i = 0; i < 16; i++) {
        if (a.get_gpr(static_cast<arm::GPR>(i)) != b.get_gpr(static_cast<arm::GPR>(i))) {
            LOG_ERROR("%s r%d mismatch at %08x: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", i, a.get_gpr(arm::GPR::PC), a.get_gpr(static_cast<arm::GPR>(i)), b.get_gpr(static_cast<arm::GPR>(i)));
        }
    }

    if (a.get_cpsr().data != b.get_cpsr().data) {
        LOG_ERROR("%s cpsr mismatch at %08x: expected: %08x got: %08x", arch == arm::Arch::ARMv5 ? "arm9" : "arm7", b.get_gpr(arm::GPR::PC),  a.get_cpsr().data, b.get_cpsr().data);
    }
}

void run_and_compare_cpus(arm::CPU& a, arm::CPU& b, int cycles) {
    for (int i = 0; i < cycles; i++) {
        a.run(1);
        b.run(1);
        compare_states(a.get_arch(), a, b);
    }
}

void run_gba(char *path) {
    gba::System a_system;
    gba::System b_system;

    a_system.set_update_callback([](f32) {});
    b_system.set_update_callback([](f32) {});

    arm::Config a_config;
    a_config.block_size = 1;
    a_config.backend_type = arm::BackendType::IRInterpreter;
    a_config.optimisations = false;

    arm::Config b_config;
    b_config.block_size = 1;
    b_config.backend_type = arm::BackendType::Jit;
    b_config.optimisations = false;

    a_system.configure_cpu_backend(a_config);
    b_system.configure_cpu_backend(b_config);

    a_system.set_game_path("");
    a_system.set_boot_mode(common::BootMode::Regular);
    a_system.reset();

    b_system.set_game_path("");
    b_system.set_boot_mode(common::BootMode::Regular);
    b_system.reset();

    auto& a_cpu = a_system.cpu;
    auto& b_cpu = b_system.cpu;

    while (true) {
        run_and_compare_cpus(*a_cpu, *b_cpu, 1);

        a_system.scheduler.tick(1);
        a_system.scheduler.run();

        b_system.scheduler.tick(1);
        b_system.scheduler.run();
    }
}

void run_nds(char *path) {
    nds::System a_system;
    nds::System b_system;

    a_system.set_update_callback([](f32) {});
    b_system.set_update_callback([](f32) {});

    arm::Config a_config;
    a_config.block_size = 32;
    a_config.backend_type = arm::BackendType::IRInterpreter;
    a_config.optimisations = true;

    arm::Config b_config;
    b_config.block_size = 32;
    b_config.backend_type = arm::BackendType::Jit;
    b_config.optimisations = true;

    a_system.configure_cpu_backend(a_config);
    b_system.configure_cpu_backend(b_config);

    a_system.set_game_path(path);
    a_system.set_boot_mode(common::BootMode::Fast);
    a_system.reset();

    b_system.set_game_path(path);
    b_system.set_boot_mode(common::BootMode::Fast);
    b_system.reset();

    auto& a_arm7 = a_system.arm7;
    auto& a_arm9 = a_system.arm9;
    auto& b_arm7 = b_system.arm7;
    auto& b_arm9 = b_system.arm9;

    while (true) {
        run_and_compare_cpus(a_arm9.get_cpu(), b_arm9.get_cpu(), 2);
        run_and_compare_cpus(a_arm7.get_cpu(), b_arm7.get_cpu(), 1);

        a_system.scheduler.tick(1);
        a_system.scheduler.run();

        b_system.scheduler.tick(1);
        b_system.scheduler.run();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        LOG_TODO("yuugen_tests: no game path was supplied");
        return 1;
    }

    std::string path = argv[1];

    auto extension = path.substr(path.find_last_of(".") + 1, path.size());
    if (extension == "gba") {
        run_gba(argv[1]);
    } else if (extension == "nds") {
        run_nds(argv[1]);
    } else {
        LOG_TODO("unhandled game extension %s", extension.c_str());
    }

    return 0;
}