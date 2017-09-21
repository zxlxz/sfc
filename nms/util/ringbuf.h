#pragma once

#include <nms/core/type.h>
#include <nms/core/memory.h>
#include <nms/core/exception.h>

namespace  nms
{

template<class T>
class Ringbuf final
{
public:
    class EEmpty final
        : public IException
    {};

    class EOverflow final
        : public IException
    {};

    explicit Ringbuf(u64 capicity)
        : data_(mnew<T>(capicity)), cap_(capicity)
    {}

    ~Ringbuf() {
        if (data_ != nullptr) {
            mdel(data_);
        }
    }

    // 0123456789
    // ^         ^
    // |         |
    template<class U>
    Ringbuf& push(U&& val) {
        if (isFull()) {
            NMS_THROW(EOverflow());
        }
        new(&data_[top_++])T(fwd<U>(val));
        if (top_ > cap_) top_ = 0;
        return *this;
    }

    T pop() {
        if (isEmpty()) {
            NMS_THROW(EEmpty());
        }
        auto tmp(move(data_[tail_++]));
        if (tail_ >= cap_) tail_ = 0;
        return tmp;
    }

    u64 len() const noexcept {
        auto result = top_ > tail_ ? top_ - tail_ : cap_ - (cap_ - top_);
        return result;
    }

    bool isFull() const noexcept {
        return len() == cap_;
    }

    bool isEmpty() const noexcept {
        return len() == 0;
    }

    Ringbuf& operator<<(T&& val) {
        push(move(val));
        return *this;
    }

    Ringbuf& operator>>(T&  val) {
        val = pop();
        return *this;
    }

private:
    T*  data_    = nullptr;
    u64 cap_    = 0;
    u64 top_    = 0;
    u64 tail_   = 0;
};

}
