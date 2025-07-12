#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "unix/alloc.inl"
#endif

#ifdef _WIN32
#include "windows/alloc.inl"
#endif
