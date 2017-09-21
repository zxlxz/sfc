#pragma once

#include <nms/core/base.h>

namespace nms
{
class StackInfo;

class IException
{
public:
    virtual ~IException() = default;

    virtual void format(IString& /*buf*/) const
    {}

    NMS_API static void        set_stackinfo();
    NMS_API static StackInfo&  get_stackinfo();

protected:
    constexpr IException() = default;
};

#define NMS_THROW(...)  IException::set_stackinfo(); throw(__VA_ARGS__)

template<class ...T>
void sformat(IString& buf, const StrView& fmt, const T& ...t);

class ESystem
    : public IException
{
public:
    using Teid      = i32;

    ESystem()
        : eid_(system_error())
    {}

    explicit ESystem(Teid id)
        : eid_(id)
    {}

    Teid eid() const noexcept {
        return eid_;
    }

    NMS_API void format(IString& buf) const override;

protected:
    Teid eid_;

    static NMS_API i32 system_error();
};

class IEOutOfRange
    : public IException
{};

template<class T>
class EOutOfRange
    : public IEOutOfRange
{
public:
    EOutOfRange(const T& min_val, const T& max_val, const T& val)
        : min_(min_val), max_(max_val), val_(val)
    { }

    void format(IString& buf) const override {
        sformat(buf, "range=[{}, {}], value=`{}`", min_, max_, val_);
    }
protected:
    T   min_;
    T   max_;
    T   val_;
};

class IEunexpect
    : public IException
{};

template<class T>
class Eunexpect
    : public IEunexpect
{
public:
    Eunexpect(const T& expect, const T& value)
        : expect_(expect), value_(value)
    { }

    void format(IString& buf) const override {
        sformat(buf, "expect=`{}`, value=`{}`", expect_, value_);
    }

protected:
    T   expect_;
    T   value_;
};

}
