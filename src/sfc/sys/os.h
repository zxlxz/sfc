#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/os.inl"
#endif

#ifdef _WIN32
#include "sfc/sys/windows/os.inl"
#endif
