#pragma once

#ifdef __unix__
#include "unix/thread.inl"
#endif

#ifdef _WIN32
#include "windows/thread.inl"
#endif