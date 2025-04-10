#pragma once

#ifdef __unix__
#include "unix/time.inl"
#endif

#ifdef _WIN32
#include "windows/time.inl"
#endif
