#pragma once

#ifdef __unix__
#include "unix/alloc.inl"
#endif

#ifdef _WIN32
#include "windows/alloc.inl"
#endif
