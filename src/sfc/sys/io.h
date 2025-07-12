#pragma once

#if defined(__unix__) || defined(__APPLE__)
#include "unix/io.inl"
#endif

#ifdef _WIN32
#include "windows/io.inl"
#endif
