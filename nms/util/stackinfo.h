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

    /*! capture current stackinfo */
    StackInfo(u32 num_frames_to_skip=0) {
        init();

        if (count_ > num_frames_to_skip) {
            count_ -= num_frames_to_skip;
        }
    }

    /*! get stack-frame count */
    u32 count() const {
        return count_;
    }

    /*! get stack-frame */
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