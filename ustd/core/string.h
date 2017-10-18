#pragma once

#include <ustd/core/vec.h>
#include <ustd/core/option.h>

namespace ustd::vec
{

using option::Option;

template<>
class Vec<char, 0>: public Slice<char>
{
  public:
    using Slice<char>::_data;
    using Slice<char>::_size;
    using Slice<char>::_capacity;

  public:    
    // method: clear
    fn clear() -> void {
        Slice::_size = 0;
    }

    // method: push
    fn push(char c) noexcept -> bool {
        if (_size + 1 >= _capacity) { return false; }
        _data[_size++] = c;
        return true;
    }

    // method: pop
    fn pop () noexcept -> Option<char> {
        using Option = Option<char>;
        return _size != 0 ? Option::Some(_data[--_size]) : Option::None();
    }

    // method: push str
    fn push_str(Slice<char> v) noexcept -> bool {
        return push_str(v.data(), v.len());
    }

    // method: push str
    fn push_str(Slice<const char> v) noexcept -> bool {
        return push_str(v.data(), v.len());
    }

    // method: push str
    template<u32 N>
    fn push_str(const char (&s)[N]) noexcept -> bool {
        return push_str(s, N - 1);
    }

  protected:
    Vec(char* ptr, u32 length, u32 capacity) noexcept
        : Slice<char>{ ptr, length, capacity }
    {}

    fn push_str(const char* s, u32 n) -> bool {
        if (_size + n > _capacity) { return false; }

        let p = _data + _size;
        for(u32 i = 0; i < n; ++i) { p[i] = s[i]; }
        _size += n;

        return true;
    }
};

template<usize N>
class Vec<char, N>: public Vec<char>
{
  public:
    using Slice<char>::_data;
    using Slice<char>::_size;
    using Slice<char>::_capacity;
    char _buf[N];

    // constructor[default]
    Vec() noexcept
        : Vec<char, 0>{ _buf, 0, u32(N) }
    {}

    // constructor[move]
    Vec(Vec&& v)noexcept : Vec() {
        this->push_str(v);
    }

    // constructor[copy]
    fn dup() const -> Vec {
        mut v = Vec();
        v.push_str(*this);
        return v;
    }

  private:
    Vec(const Vec&)             = delete;
    Vec& operator=(const Vec&)  = delete;
};

}

namespace ustd::string
{
using String = ustd::vec::Vec<char>;
}