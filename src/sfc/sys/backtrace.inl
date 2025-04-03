#pragma once

#ifdef __unix__
#include "unix/backtrace.inl"
#endif

#ifdef _WIN32
#include "windows/backtrace.inl"
#endif
