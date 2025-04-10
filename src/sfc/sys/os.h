#pragma once

#ifdef __unix__
#include "unix/os.inl"
#endif

#ifdef _WIN32
#include "windows/os.inl"
#endif
