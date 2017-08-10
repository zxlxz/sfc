#pragma once

#include <nms/core.h>
#include <nms/math.h>
#include <nms/cuda/runtime.h>
#include <nms/cuda/engine.h>

namespace nms::cuda
{

template<class T, u32 N>
struct View;

template<class T, u32 N>
View<T,N> mkView(const nms::View<T,N>& view) {
    return view;
}

template<class T, u32 N>
struct View
    : nms::View<T, N>
{
public:
    using base = nms::View<T, N>;

    View(const base& view)
        : base(view)
    {}

    explicit View(T* ptr, const u32(&len)[N])
        : base(ptr, len)
    {}

    /* slice */
    template<class ...I, u32 ...S>
    auto slice(const I(&...idxs)[S]) const noexcept {
        return mkView(base::slice(idxs...));
    }

    /* slice */
    template<class ...I, u32 ...S>
    auto slice(const I(&...idxs)[S]) noexcept {
        return mkView(base::slice(idxs...));
    }

    /*!
     * slice the view
     * @param ss sections
     * @sa slice
     */
    template<class I, u32 S, class ...Is, u32 ...Ss>
    auto operator()(const I(&s)[S], const Is(&...ss)[Ss]) noexcept {
        return slice(s, ss...);
    }

    /*!
     * slice the view
     * @param ss sections
     * @sa slice
     */
    template<class I, u32 S, class ...Is, u32 ...Ss>
    auto operator()(const I(&s)[S], const I(&...ss)[Ss]) const noexcept {
        return slice(s, ss...);
    }
};

template<class T, u32 N>
constexpr ForeachExecutor mkForeachExecutor(const View<T, N>&) {
    return {};
}

template<class T, u32 N>
nms::View<T, N> toLambda(const View<T, N>& v) {
    return v;
}

namespace device
{
template<class T, u32 N>
class Array
    : public View<T, N>
    , public INocopyable
{
public:
    using base = View<T, N>;

    explicit Array(const u32(&len)[N])
        : base(nullptr, len) {
        auto dat = mnew<T>(base::numel());
        base::data_ = dat;
        deleter_ = delegate<void()>([=] { mdel(dat); });
    }

    virtual ~Array() {
        deleter_();
        base::data_ = nullptr;
    }

    Array(Array&& rhs) noexcept
        : base(static_cast<base&&>(rhs)) {
        base::_dat = nullptr;
    }

    Array& operator=(Array&& rhs) noexcept {
        base::operator  = (rhs);
        deleter_        = move(rhs.deleter_);
        return *this;
    }

protected:
    Array(const Array& rhs)
        : Array(rhs.len()) {
        *this <<= rhs;
    }

private:
    delegate<void()>    deleter_;
};
}

namespace host
{

template<class T, u32 N>
class Array
    : public View<T, N>
    , public INocopyable
{
public:
    using base = View<T, N>;

    explicit Array(const u32(&len)[N])
        : base(nullptr, len) {
        auto dat = hnew<T>(base::numel());
        base::data_ = dat;
        deleter_ = [=] { hdel(dat); };
    }

    virtual ~Array() {
        deleter_();
        base::data_ = nullptr;
    }

    Array(Array&& rhs) noexcept
        : base(static_cast<base&&>(rhs)) {
        base::_dat = nullptr;
    }

    Array& operator=(Array&& rhs) noexcept {
        base::operator  = (rhs);
        deleter_        = move(rhs.deleter_);
        return *this;
    }

protected:
    Array(const Array& rhs)
        : Array(rhs.len()) {
        *this <<= rhs;
    }

private:
    delegate<void()>    deleter_;
};

}

}
