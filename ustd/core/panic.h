#pragma once

#include <ustd/core/_.h>
#include <ustd/core/str.h>

namespace ustd
{

struct PanicInfo
{};

fn panic() -> void;

fn panic(str fmt) -> void;

template<class ...T>
fn panic(str fmt, const T& ...args) -> void;

}
