#pragma once

#include <ustd/core.h>

namespace ustd
{
namespace boxed
{

template<class T>
class Box final
{
    using P = T*;

    mutable T*  _ptr;

    Box(T* ptr): _ptr(ptr)
    {}

  public:
    Box()
        : _ptr{nullptr}
    {}

    template<typename ...U>
    Box(U&& ...u)
        : Box{new T(as_fwd<U>(u)...)}
    {}

    Box(Box&& other): _ptr(other._ptr) {
        other._ptr = nullptr;
    }

    ~Box() {
        if (_ptr==nullptr) {
            return;
        }
        delete _ptr;
    }

    Box(const Box&) = delete;
    fn operator=(const Box&) = delete;

    static fn from_raw(P&& ptr) -> Box {
        return Box(ptr);
    }

    fn operator *() noexcept -> T& {
        return *_ptr;
    }

    fn operator *() const noexcept -> T& {
        return *_ptr;
    }

    void forget() const {
        _ptr = nullptr;
    }
};

}

}
