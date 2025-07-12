#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "unix/os.inl"
#endif

#ifdef _WIN32
#include "windows/os.inl"
#endif
