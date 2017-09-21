#include <nms/test.h>
#include <nms/util/library.h>
#include <nms/io/path.h>

#ifdef NMS_OS_WINDOWS
extern "C"
{
    static auto    RTLD_LAZY = 0x1;
    static void*   INVALID_HANDLE_VALUE = reinterpret_cast<void*>(0ll - 1ll);
    void*          LoadLibraryA(const char* path);
    int            FreeLibrary(void* handle);
    void*          GetProcAddress(void* handle, const char* name);
}

static void* dlopen(const char* path, int mode) {
    auto ret = ::LoadLibraryA(path);
    (void)mode;
    if (ret == INVALID_HANDLE_VALUE) {
        ret = nullptr;
    }
    return ret;
}

static void* dlsym(void* handle, const char* symbol) {
    auto ret = GetProcAddress(handle, symbol);
    return ret;
}

static int dlclose(void* handle) {
    auto ret = ::FreeLibrary(handle);
    return ret;
}
#endif

namespace nms
{

NMS_API Library::Library(const StrView& file_name) {
    auto path       = io::Path(file_name);
    auto full_path  = path.path();

    object_ = dlopen(full_path.data(), RTLD_LAZY);

    if (object_ ==  nullptr) {
        io::log::error("nms.Library: load {} failed", path);
    }
}

NMS_API Library::~Library() {
    if (object_ == nullptr) {
        return;
    }
    dlclose(object_);
}

NMS_API Library::Function Library::operator[](StrView name) const {
    if (object_ == nullptr || name.count()==0) {
        return { nullptr };
    }
    auto ptr = dlsym(object_, name.data());
    return Function{ reinterpret_cast<void*>(ptr) };
}

}