#pragma once

#include <ustd/core/slice.h>
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
    Vec(Vec&& v)noexcept : Vec() {
        Slice<T>::_size = v._size;
        v._size = 0;

        for(u32 i = 0; i < Slice<T>::_size; ++i) {
            new(&_buf[i])T(as_mov(v._buf[i]));
        }
    }
};

template<typename T>
class Vec<T, 0>: public Slice<T>
{
  protected:
    Vec(T* ptr, u32 length, u32 capacity) noexcept
        : Slice<T> { ptr, length, capacity }
    {}
       
  public:
    // method: clear
    fn clear() {
        // do: clear elements
        if (!$is<$value, T>) {
            for (u32 i = 0; i < Slice<T>::_size; ++i) {
                Slice<T>::_data[i].~T();
            }
        }
        Slice<T>::_size = 0;
    }

    // method: push
    template<typename U>
    fn push(U&& u) noexcept -> bool {
        if (Slice<T>::_size + 1 >= Slice<T>::_capacity) {
            return false;
        }
        let ptr = &Slice<T>::_data[Slice<T>::_size++];
        $new(ptr, as_fwd<U>(u));

        return true;
    }

    // method: pop
    fn pop () noexcept -> Option<T> {
        if (this->_size == 0) {
            return {};
        }
        let ret = Option<T>{as_mov(Slice<T>::_data[Slice<T>::_size]) };
        return ret;
    }
};

}
