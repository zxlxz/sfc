#pragma once

#include <ustd/core/view.h>
#include <ustd/core/option.h>

namespace ustd::vec
{

using option::Option;
    
template<typename T, usize N = 0>
class Vec;

template<typename T, usize N>
class Vec: public Vec<T, 0>
{
    union {
        u8  _nul;
        T   _buf[N];
    };

    Vec(const Vec&)             = delete;    
    Vec& operator=(const Vec&)  = delete;

  public:

    // constructor[default]
    Vec() noexcept
        : Vec<T, 0>{_buf, 0, u32(N)}, _nul(0)
    {}

    // constructor[move]
    Vec(Vec&& v) noexcept : Vec() {
        View<T>::_size = v._size;
        v._size = 0;

        for(u32 i = 0; i < View<T>::_size; ++i) {
            new(&_buf[i])T(as_mov(v._buf[i]));
        }
    }
};

template<typename T>
class Vec<T, 0>: public View<T>
{
    using base = View<T>;

  protected:
    Vec(T* ptr, u32 length, u32 capacity) noexcept
        : base{ ptr, length, capacity }
    {}
       
  public:
    // method: clear
    fn clear() {
        // do: clear elements
        if (!$is<$value, T>) {
            for (u32 i = 0; i < View<T>::_size; ++i) {
                base::_data[i].~T();
            }
        }
        base::_size = 0;
    }

    // method: push
    template<typename U>
    fn push(U&& u) noexcept -> bool {
        if (base::_size + 1 >= base::_capacity) {
            return false;
        }
        let ptr = &base::_data[base::_size++];
        $new(ptr, as_fwd<U>(u));

        return true;
    }

    // method: pop
    fn pop () noexcept -> Option<T> {
        if (this->_size == 0) {
            return {};
        }
        let ret = Option<T>{as_mov(base::_data[base::_size]) };
        return ret;
    }
};

}
