#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_OSX
#else
static_assert(false, "Linux and windows support is not supported yet");
#endif