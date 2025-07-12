#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "unix/thread.inl"
#endif

#ifdef _WIN32
#include "windows/thread.inl"
#endif
