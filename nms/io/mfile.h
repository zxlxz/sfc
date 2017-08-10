#pragma once

#include <nms/core.h>
#include <nms/io/path.h>

namespace nms::io
{

/**
 * memory mapped file
 */
class MFile final: public INocopyable
{
public:
    NMS_API explicit MFile(const Path& path, u64 size=0);
    NMS_API ~MFile();

    u64         size()  const noexcept { return size_;  }
    const void* data()  const noexcept { return data_;   }
    void*       data()        noexcept { return data_;   }

private:
    int     obj_     = -1;
    void*   data_    = nullptr;
    u64     size_    = 0;
};

}
