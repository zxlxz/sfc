#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#endif

#ifdef _WIN32
#include "sfc/sys/windows/alloc.inl"
#endif
