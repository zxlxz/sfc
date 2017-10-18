#pragma once

#include <ustd/core/slice.h>

namespace ustd
{

using str = Slice<const char,0>;
using strs= Slice<str>;

template<>
struct Slice<const char, 0>
{
    const char* _data;
    u32         _size;
    u32         _capacity;

    Slice()
        : _data{nullptr}, _size{0}, _capacity{0}
    {}

    Slice(const char* s, u32 len, u32 capacity=0)
        : _data{s}, _size{len}, _capacity{capacity}
    {}

    template<usize N>
    Slice(const char(&v)[N]): Slice{v, N-1}
    {}
    
   static fn from_raw(const char* s)->Slice;

    // operator[]
    fn operator[](u32 i) const noexcept -> char {
        return _data[i];
    }

    // property[r]: data
    fn data() const noexcept -> const char* {
        return _data;
    }

    // property[r]: len
    fn len() const noexcept -> u32 {
        return _size;
    }

    // property[r]: capacity
    fn capacity() const noexcept -> u32 {
        return _capacity;
    }

    // iterator[const]: begin
    fn begin() const noexcept -> const char* {
        return _data;
    }

    // iterator[const]: begin
    fn end() const noexcept -> const char* {
        return _data + _size;
    }

    // method: slice
    template<class I>
    fn operator()(I begin, I end) const -> str {
        let pos_begin = begin >= 0 ? u32(begin) : u32(_size - begin);
        let pos_end   = end   >= 0 ? u32(end  ) : u32(_size - end  );
        return str{ _data + pos_begin, pos_end - pos_begin + 1 };
    }

    // method: ==
    fn operator==(const str& other) -> bool {
        if (this->_size != other._size) {
            return false;
        }
        for (u32 i = 0; i < this->_size; ++i) {
            if (_data[i] != other._data[i]) {
                return false;
            }
        }

        return true;
    }

    // method: !=
    fn operator!=(const str& other) -> bool {
        return !this->operator==(other);
    }

    // method: starts_with
    fn starts_with(str pat) -> bool {
        if (pat._size > this->_size ) {
            return false;
        }
        let sub_str = str(_data, pat._size);
        return pat == sub_str;
    }

    // method: starts_with
    fn ends_with(str pat) -> bool {
        if (pat._size > this->_size) {
            return false;
        }
        let sub_str = str(_data + _size - pat._size, pat._size);
        return pat == sub_str;
    }
};

template<class T>
fn type_name() -> str {

#if defined(_MSC_VER)
    let prefix_len = sizeof("struct ustd::slice<char const, 0> __cdecl ustd::type_name<struct ") - 1;
    let suffix_len = sizeof("::_>(void)") - 1;
    return { __FUNCSIG__ + prefix_len, u32(sizeof(__FUNCSIG__) - prefix_len - suffix_len - 1) };
#else
    let prefix_len = sizeof("str ustd::type_name() [T = ") - 1;
    let suffix_len = sizeof("::_]") - 1;
    return { __PRETTY_FUNCTION__ + prefix_len, u32(sizeof(__PRETTY_FUNCTION__) - prefix_len - suffix_len - 1) };
#endif

}

}
