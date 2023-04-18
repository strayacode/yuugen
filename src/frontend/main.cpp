#include "common/logger.h"
#include "core/core.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        logger.error("main: incorrect amount of arguments supplied");
        return 0;
    }

    core::Core core;
    core.set_game_path(argv[1]);
    core.start();
    return 0;
}