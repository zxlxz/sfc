#pragma once

#include "sfc/core.h"

namespace sfc::sys {

namespace posix {}
namespace windows {}

#if defined(__unix__) || defined(__APPLE__)
using namespace posix;
#endif

#ifdef _WIN32
using namespace windows;
#endif

}  // namespace sfc::sys
