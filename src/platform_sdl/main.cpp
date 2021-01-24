#include <stdio.h>
#include "host_interface.h"
#include <memory>
#include <common/log.h>

int main(int argc, char *argv[]) {
	std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();

	if (argc < 2) {
        log_fatal("no rom argument or other arguments were specified!");
    }

	if (!host_interface->initialise()) {
		// do cleanup and shutdown emulator
		host_interface->cleanup();
	}
	int i;
	for (i = 1; i < argc - 1; i++) {
        printf("%s\n", argv[i]);
    }
    
	host_interface->run(argv[i]);

	return EXIT_SUCCESS;
}