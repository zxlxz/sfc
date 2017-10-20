#pragma once

namespace ustd::os
{

namespace windows
{}

namespace linux
{}

namespace apple
{}

#ifdef _WIN32
using namespace windows;
#endif

#ifdef __linux
using namespace linux;
#endif

#ifdef __APPLE__
using namespace apple;
#endif

}
