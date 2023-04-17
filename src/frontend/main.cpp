#include "core/core.h"

int main(int argc, char *argv[]) {
    core::Core core;
    core.set_game_path(argv[1]);
    core.start();
    return 0;
}