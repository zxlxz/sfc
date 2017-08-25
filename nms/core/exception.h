#pragma once

#include <nms/core/base.h>
#include <nms/util/stacktrace.h>

namespace nms
{

class IException;
class ESystem;

class CallStacks;
NMS_API const CallStacks& gExceptionStacks();
NMS_API void gSetExceptionStacks(CallStacks&&);

#define NMS_THROW(...)  nms::gSetExceptionStacks(nms::CallStacks{}); throw(__VA_ARGS__)

class ESystem: public IException
{
public:
    ESystem()
        : id_(current())
    {}

    explicit ESystem(i32 id)
        : id_(id)
    {}

    i32 id() const noexcept {
        return id_;
    }

#ifndef _M_CEE
    NMS_API void format(String& buf) const override;
#endif

protected:
    i32 id_;

    static NMS_API u32 current();
};

class EBadType: public IException
{};

class EBadSize: public IException
{};

class EInvalidValue: public IException
{};

class EOutOfRange: public IException
{};

}
