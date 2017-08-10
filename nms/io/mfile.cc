#include <nms/config.h>
#include <nms/io/file.h>
#include <nms/io/mfile.h>
#include <nms/io/log.h>

namespace nms::io
{

#ifdef NMS_OS_WINDOWS
extern "C" {
    int   CloseHandle(void* handle);
    void* CreateFileMappingA(void* hFile, void* attributes, u32 flProtect, u32 dwMaximumSizeHigh, u32 dwMaximumSizeLow, const char* lpName);
    void* MapViewOfFile(void* hFileMappingObject, u32 dwDesiredAccess, u32 dwFileOffsetHigh, u32 dwFileOffsetLow, u64 dwNumberOfBytesToMap);
    int   UnmapViewOfFile(void* lpBaseAddress);
}
#endif

MFile::MFile(const Path& path, u64 size)
    : size_(size)
{
    // open/create file
    obj_ = ::open(path.cstr(), O_CREAT);
    const auto eid = errno;

    if (eid !=0 || obj_ < 0) {
        log::error("nms.io.MFile: cannot open/create {}", path);
        throw ESystem(eid);
    }

    stat_t st;
    ::fstat(obj_, &st);

#ifdef NMS_CC_MSVC
    // map
    const u32 size_high = (size >> 32);
    const u32 size_low  = (size << 32) >> 32;

    auto hfile = reinterpret_cast<void*>(_get_osfhandle(obj_));
    auto hmmap = CreateFileMappingA(hfile, nullptr, 0x04, size_high, size_low, nullptr);

    // view
    data_ = MapViewOfFile(hmmap, 0x0002, 0, 0, size);
    if (data_ == nullptr) {
        log::error("nms.io.MFile: map view of file failed");
        throw ESystem();
    }

    // finalize
    if (hmmap != reinterpret_cast<void*>(static_cast<long>(-1))) {
        CloseHandle(hfile);
    }
#else
    data_ = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, obj_, 0);
#endif
}

MFile::~MFile() {
    if (data_ != nullptr) {
#ifdef NMS_CC_MSVC
        UnmapViewOfFile(data_);
#else
        munmap(data_, size_);
#endif
    }
    if (obj_ != -1 ) {
        ::close(obj_);
    }
}

}

