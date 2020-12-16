#include <emulator/emulator.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    Emulator emulator;
    if (argc < 2) {
        printf("no rom argument or other arguments were specified!\n");
        return 0;
    }
    int i;
    for (i = 1; i < argc - 1; i++) {
        printf("%s\n", argv[i]);
    }
    
    emulator.run(argv[i]);
    
    return 0;
}