#pragma once

#include "sfc/core.h"

namespace sfc::sys {

namespace posix {}
namespace windows {}

#ifdef __unix__
using namespace posix;
#endif

#ifdef _WIN32
using namespace windows;
#endif

}  // namespace sfc::sys
