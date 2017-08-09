#pragma once

#include <nms/core.h>

namespace nms
{

class ProcStacks
{
public:
    ProcStacks() {
        init();
    }

    u32 count() const {
        return count_;
    }

    NMS_API String operator[](u32 idx) const;

protected:
    u32     count_  = 0;
    void*   stacks_[128];
    NMS_API void init();
};

}