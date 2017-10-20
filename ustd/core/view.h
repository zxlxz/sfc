#pragma once

#include <ustd/core/type.h>
#include <ustd/core/iter.h>

extern "C"
{
    ustd::usize strlen(char const* str);
}

namespace ustd
{

template<typename T, usize N=0>
struct View;

template<typename T>
struct View<T, 0>
{
    T*  _data;
    u32 _size;
    u32 _capacity;

    // constructor:
    View() noexcept
        : _data{nullptr}, _size{0}, _capacity{0}
    {}

    // constructor:
    View(T* ptr, u32 length)
        : _data{ ptr }, _size{ length }, _capacity{ length }
    {}

    // constructor:
    View(T* ptr, u32 length, u32 capacity)
        : _data{ptr}, _size{length}, _capacity{capacity}
    {}

    template<u32 N, class=$when<$is<const char, T> && N!=0> >
    View(const char(&s)[N])
        : _data{s}, _size{N-1}, _capacity{N-1}
    {}

    template<typename Tchar, class=$when<$is<const char, T> && $is<char, Tchar> > >
    static fn from_raw(const Tchar* s)-> View {
        if (s == nullptr) {
            return {};
        }
        let n = ::strlen(s);
        return { s, u32(n) };
    }

    // convert: const
    operator View<const T>() const noexcept {
        return { as_const(_data), _size, _capacity };
    }

    // operator: []
    fn operator[](u32 i) const noexcept -> T& {
        return _data[i];
    }

    // method: View
    template<class I>
    fn slice(I begin, I end) const noexcept -> View<T, 0> {
        let beg_pos = begin >= 0 ? u32(begin) : _size - u32(0 - begin);
        let end_pos = end   >= 0 ? u32(end  ) : _size - u32(0 - end  );
        return { _data + beg_pos, end_pos - beg_pos + 1, 0 };
    }

    // property[r]: data
    fn data() const noexcept -> T* {
        return _data;
    }

    // property[r]: len
    fn len() const noexcept -> u32 {
        return _size;
    }

    // property[r]: capacity
    fn capacity() const noexcept -> u32{
        return _capacity;
    }

    // property[r]: is_empty 
    fn is_empty() const noexcept {
        return _size == 0;
    }

    // method: equal
    fn operator==(const View& other) const {
        if (other._size != _size) {
            return false;
        }
        if (other._data == _data) {
            return true;
        }
        for (u32 i = 0; i < _size; ++i) {
            if (_data[i] != other._data[i]) {
                return false;
            }
        }
        return true;
    }

    // method: not equal
    fn operator!=(const View& other) const {
        return !(*this == other);
    }

    // method: starts_with
    fn starts_with(const View& needle) {
        if (needle._size > _size) {
            return false;
        }
        let prefix = View(0u, needle._size - 1);
        return prefix == needle;
    }

    // method: starts_with
    fn ends_with(const View& needle) {
        if (needle._size > _size) {
            return false;
        }
        let suffix = View(_size - 1 - needle._size, _size - 1);
        return suffix == needle;
    }

    // method: contains
    fn contains(const T& iterm) -> bool {
        for (u32 i = 0; i < _size; ++i) {
            if (_data[i] == iterm) {
                return true;
            }
        }
        return false;
    }

    // iter:
    fn into_iter() const -> Iter<T> {
        return { _data, _size };
    }
};

using i8s = View<i8>;
using u8s = View<u8>;

using i16s = View<i16>;
using u16s = View<u16>;

using i32s = View<i32>;
using u32s = View<u32>;

using i64s = View<i64>;
using u64s = View<u64>;

using f32s = View<f32>;
using f32s = View<f32>;

}
