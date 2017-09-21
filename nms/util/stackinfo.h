#pragma once

#include <nms/core/base.h>

namespace nms
{

class StackInfo
{
public:
    struct Frame
    {
        void* ptr;
        NMS_API void format(IString& buf) const;
    };


    StackInfo(u32 skip_cnt=0) {
        init();

        if (count_ > skip_cnt) {
            count_ -= skip_cnt;
        }
    }

    u32 count() const {
        return count_;
    }

    Frame operator[](u32 idx) const {
        if (idx+3 >= count_) {
            return Frame{ nullptr };
        }
        return Frame{ stacks_[idx+3] };
    }

    NMS_API void format(IString& buf) const;

protected:
    void*   stacks_[64];
    u32     count_ = 0;

    NMS_API void init();
};

}