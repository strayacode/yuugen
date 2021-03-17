#include <memory>
#include "host_interface.h"
#include <util/log.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        log_fatal("no arguments were given!");
    }

    std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();
   
    if (!host_interface->Initialise()) {
        host_interface->Cleanup();
    }
    int i = 1;
    for (int i = 1; i < argc - 1; i++) {
        printf("%s\n", argv[i]);
    }
    host_interface->Run(argv[i]);

    return 0;
}
