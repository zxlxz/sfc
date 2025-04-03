#pragma once

#ifdef __unix__
#include "unix/env.inl"
#endif

#ifdef _WIN32
#include "windows/env.inl"
#endif
