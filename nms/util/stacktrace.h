#pragma once

#include <nms/config.h>

namespace nms
{

class String;

class CallStacks
{
public:
    CallStacks() {
        init();
    }

    u32 count() const {
        if (count_ > 4) {
            return count_ - 4;
        }
        return 0;
    }

    struct Stack
    {
        void* ptr;
        NMS_API void format(String& buf) const;
    };

    Stack operator[](u32 idx) const {
        if (idx + 2 >= count_) {
            return Stack{ nullptr };
        }

        return Stack{ stacks_[idx+2] };
    }

protected:
    u32     count_  = 0;
    void*   stacks_[128];
    NMS_API void init();
};

}