#include <stdlib.h>
#include <string.h>

#include <memory>
#include "Frontend/HostInterface.h"

int main() {
    std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();

    if (!host_interface->initialise()) {
        host_interface->Shutdown();
        return 0;
    }
    
    host_interface->Run();
    host_interface->Shutdown();

    return 0;
}