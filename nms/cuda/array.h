#pragma once

#include <nms/core.h>
#include <nms/math.h>
#include <nms/cuda/runtime.h>
#include <nms/cuda/engine.h>
#include <nms/cuda/vrun.h>

namespace nms::cuda
{

namespace device
{
template<class T, u32 N>
class Array
    : public View<T, N>
{
public:
    using base  = View<T, N>;
    using Tvrun = Vrun;
    using Tsize = typename base::Tsize;

    explicit Array(const Tsize(&len)[N])
        : base(nullptr, len)
    {
        const auto cnt = base::count();
        if (cnt != 0) {
            base::data_ = mnew<T>(cnt);
        }
    }

    virtual ~Array() {
        if (base::data_!=nullptr) {
            mdel(base::data_);
        }
        base::data_ = nullptr;
    }

    Array(Array&& rhs) noexcept
        : base(rhs) {
        rhs.data_ = nullptr;
    }

    Array dup() const {
        auto tmp(*this);
        return tmp;
    }

    Array& operator=(Array&& rhs) noexcept {
        if (this != &rhs) {
            {
                auto tmp(move(*this));
            }

            base::operator=(rhs);
            rhs.data_ = nullptr;
        }

        return *this;
    }

    Array& operator=(const Array&) = delete;

protected:
    Array(const Array& rhs)
        : Array(rhs.len()) {
        *this <<= rhs;
    }
};
}

namespace host
{

template<class T, u32 N>
class Array
    : public View<T, N>
{
public:
    using base  = View<T, N>;
    using Tvrun = cuda::Vrun;
    using Tsize = typename base::Tsize;

    constexpr static const auto $rank = base::$rank;

    explicit Array(const Tsize(&size)[$rank])
        : base(nullptr, size) {
        const auto cnt    = base::count();
        if (cnt > 0) {
            base::data_ = hnew<T>(base::count());
        }
    }

    virtual ~Array() {
        if (base::data_ != nullptr) {
            hdel(base::data_);
        }
        static_cast<base&>(*this) = {};
    }

    Array(Array&& rhs) noexcept
        : base(rhs) {
        rhs.data_ = nullptr;
    }

    Array dup() const {
        auto tmp(*this);
        return tmp;
    }

    Array& operator=(Array&& rhs) noexcept {
        if (this != &rhs) {
            {
                auto tmp(move(*this));
            }

            base::operator=(rhs);
            rhs.data_ = nullptr;
        }
        return *this;
    }

    Array& operator=(const Array&) = delete;

protected:
    Array(const Array& rhs)
        : Array(rhs.size()) {
        *this <<= rhs;
    }
};

}

}
