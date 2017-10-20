#pragma once

#include <ustd/core.h>
#include <ustd/math/view.h>

namespace ustd::math
{

template<typename T, usize N>
using View = ustd::View<T, N>;

template<typename T, usize N>
class Array: public View<T, N>
{
    using base = View<T, N>;

public:
    Array(const u32(&dims)[N])
        : base{nullptr, dims}
    {
        let cnt = base::count();
        base::_data = new T[cnt];
    }

    ~Array() {
        if (base::_data == nullptr) {
            return;
        }
        delete[] base::_data;
    }

    Array(Array&& other)
        : base{ other }
    {
        other._data = nullptr;
    }

    Array(const Array&) = delete;
    void operator=(const Array&) = delete;
};

}
