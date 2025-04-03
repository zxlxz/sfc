#pragma once

#ifdef __unix__
#include "unix/fs.inl"
#endif

#ifdef _WIN32
#include "windows/fs.inl"
#endif
