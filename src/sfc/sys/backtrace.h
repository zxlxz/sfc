#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "unix/backtrace.inl"
#endif

#ifdef _WIN32
#include "windows/backtrace.inl"
#endif
