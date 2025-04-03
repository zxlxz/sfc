#pragma once

#ifdef __unix__
#include "unix/sync.inl"
#endif

#ifdef _WIN32
#include "windows/sync.inl"
#endif
