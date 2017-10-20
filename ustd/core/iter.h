#pragma once

#include <ustd/core/type.h>

namespace ustd
{

struct IterEOF
{};

template<typename T>
struct Iter
{
    Iter(T* data, u32 size) : _data{ data }, _size{ size }, _idx{ 0 }
    {}

    fn count() const -> u32 {
        return _size;
    }

    fn operator *() noexcept -> T& {
        return _data[_idx];
    }

    fn operator *() const noexcept -> const T& {
        return _data[_idx];
    }

    fn operator++() -> void {
        ++_idx;
    }

    fn operator ==(IterEOF) const {
        return _idx >= _size;
    }

    fn operator !=(IterEOF) const {
        return _idx < _size;
    }

  private:
    T*  _data;
    u32 _size;
    u32 _idx;
};

template<class T>
fn begin(T&& t) -> decltype(t.into_iter()) {
    return t.into_iter();
}

template<class T>
fn end(T&& t) -> decltype(t.into_iter(), IterEOF{}) {
    return {};
}


}