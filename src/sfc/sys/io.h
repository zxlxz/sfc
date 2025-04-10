#pragma once

#ifdef __unix__
#include "unix/io.inl"
#endif

#ifdef _WIN32
#include "windows/io.inl"
#endif
