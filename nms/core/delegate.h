#pragma once

#include <nms/core/list.h>
#include <nms/core/exception.h>

namespace nms
{

template<class F>
class delegate;

namespace ns_delegate
{

template<class T>
class ILambda;

template<class T, class F>
class TLambda;

template<class R, class ...T>
class ILambda<R(T...)>
{
public:
    virtual ~ILambda()
    {}

    virtual R operator()(T ...) = 0;
};

template<class R, class ...T, class F>
class TLambda<R(T...), F>
    : public F
    , public ILambda<R(T...)>
{
public:
    TLambda(F&& f)
        : F(static_cast<F&&>(f))
    {}

    R operator()(T ...t) override {
        return F::operator()(t...);
    }
};

}

template<class R, class ...T>
class delegate<R(T...)> final
    : public INocopyable
{
public:
    class EInvalid: public IException
    {};

    constexpr delegate() noexcept
        : data_(nullptr)
    {
        buff_[sizeof(buff_) - 1] = 0;
    }

    template<class F>
    explicit delegate(F&& f) {
        using TLambda = ns_delegate::TLambda<R(T...), F>;
        buff_[sizeof(buff_) - 1] = sizeof(TLambda) < u8(0xFF) ? sizeof(TLambda) : u8(0xFF);

        if (sizeof(TLambda) < sizeof(buff_)) {
            new(buff_)TLambda(static_cast<F&&>(f));
        }
        else {
            data_ = mnew<TLambda>(1);
            new(data_)TLambda(static_cast<F&&>(f));
        }
    }

    ~delegate() {
        // empty
        if (buff_[sizeof(buff_) - 1] == 0) {
        }
        // buff
        else if (buff_[sizeof(buff_) - 1] < sizeof(buff_) ) {
            reinterpret_cast<ILambda*>(buff_)->~ILambda();
        }
        // data
        else {
            data_->~ILambda();
            mdel(data_);
        }
    }

    delegate(delegate&& rhs) noexcept {
        for (u32 i = 0; i < sizeof(buff_); ++i) {
            buff_[i]     = rhs.buff_[i];
            rhs.buff_[i] = 0;
        }
    }

    /* copy construct */
    delegate& operator=(delegate&& rhs) noexcept {
        swap(buff_, rhs.buff_);
        return *this;
    }

    /* reset */
    template<class U>
    delegate& operator=(U&& u) noexcept {
        delegate tmp(fwd<U>(u));
        nms::swap(*this, tmp);
        return *this;
    }

    template<class ...U>
    R operator()(U&& ...u) {
        return (*lambda())(fwd<U>(u)...);
    }

    __forceinline operator bool()  const noexcept {
        return buff_[sizeof(buff_) - 1] != 0;
    }

    __forceinline bool operator!() const noexcept {
        return buff_[sizeof(buff_) - 1] == 0; 
    }

private:
    using ILambda = ns_delegate::ILambda<R(T...)>;

    union
    {
        ILambda*    data_;
        u8          buff_[32];
    };

    ILambda* lambda() {
        if (buff_[sizeof(buff_) - 1] == 0) {
            NMS_THROW(EInvalid{});
        }
        if (buff_[sizeof(buff_) - 1] < sizeof(buff_) ) {
            return reinterpret_cast<ILambda*>(buff_);
        }
        else {
            return data_;
        }
    }
};

}