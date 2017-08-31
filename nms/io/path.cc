#include <nms/config.h>
#include <nms/io/path.h>
#include <nms/io/log.h>

namespace nms::io
{

extern "C" {
    void* GetModuleHandleA(const char* lpModuleName);
    u32   GetModuleFileNameA(void* hModule, char lpFileName[], u32 nSize);
}

static StrView _get_usr_dir() {
    printf("????\n");
    static char exe_path_buf[1024];
#if defined(NMS_OS_WINDOWS)
    const auto mod = GetModuleHandleA(nullptr);
    GetModuleFileNameA(mod, exe_path_buf, sizeof(exe_path_buf));
#elif defined(NMS_OS_MACOS)
    auto exe_path_len = u32(sizeof(exe_path_buf));
    _NSGetExecutablePath(exe_path_buf, &exe_path_len);
    exe_path_buf[exe_path_len] = '\0';
#elif defined(NMS_OS_POSIX)
    const auto exe_path_len = ::readlink("/proc/self/exe", exe_path_buf, sizeof(exe_path_buf));
    exe_path_buf[exe_path_len] = '\0';
#endif

    static char usr_path_buf[1024];
#ifdef NMS_OS_WINDOWS
    ::strncat(exe_path_buf, "/../../", 8);
    ::_fullpath(usr_path_buf, exe_path_buf, sizeof(usr_path_buf));
    const auto  path_len = ::strlen(usr_path_buf);
    for (auto i = 0; i < path_len; ++i) {
        if (usr_path_buf[i] == '\\') {
            usr_path_buf[i] = '/';
        }
    }
#else
    const auto  path_len = snprintf(usr_path_buf, sizeof(usr_path_buf), "%s/../../", exe_path_buf);
#endif
    return StrView{ usr_path_buf, {u32(path_len)} };
}

NMS_API void Path::init(StrView str) {
    if (str.count() >=2  && str[0] == '#' && str[1] == '/') {
        static auto usr_dir = _get_usr_dir();
        str_ = usr_dir;
        auto   sub_path = str.slice(2, -1);
        str_ += sub_path;
    }
    else {
        str_ = str;
    }
    str_.cstr();
}

NMS_API Path& Path::operator/=(const Path& rhs) {
    if (str_[-1] != '/') {
        str_ += "/";
    }
    str_ += rhs.str_;
    str_.cstr();
    return *this;
}

NMS_API Path Path::operator/(const Path& rhs) const {
    auto ret(*this);
    ret /= rhs;
    return ret;
}

NMS_API Path& Path::operator+=(const Path& rhs) {
    str_ += rhs.str_;
    str_.cstr();
    return *this;
}

NMS_API Path Path::operator+(const Path& rhs) const {
    auto ret(*this);
    ret += rhs;
    return ret;
}

NMS_API StrView Path::ext() const {
    const auto  n = str_.count();
    auto        i = n - 1;
    while (i > 0 && str_[i] != '.') --i;
    return str_.slice( i, n - 1);
}

NMS_API StrView Path::base() const {
    const auto  n = str_.count();
    auto        i = n - 1;
    while (i > 0 && str_[i] != '.') --i;
    if (i == 0) {
        return str_;
    }
    else {
        return str_.slice(0u, i - 1);
    }
}


NMS_API Path cwd() {
    char buf[256];
    auto ptr = ::getcwd(buf, sizeof(buf));

    if (ptr == nullptr) {
        return {};
    }

    const auto len = u32(::strlen(ptr));
    return Path(StrView{ ptr, {u32(len)} });
}

NMS_API void chdir(const Path& path) {
    const StrView cpath = path;
    if (cpath.count()==0) {
        log::error("nms.io.chdir: null path!!");
        NMS_THROW(ESystem(0));
    }
    const auto ret = ::chdir(cpath.data());
    
    if (ret != 0) {
        log::error("nms.io.chdir: failed");
        NMS_THROW(ESystem());
    }

}

NMS_API void mkdir(const Path& path) {
    const StrView cpath = path;

    if (cpath.count()==0) {
        log::error("nms.io.chdir: null path!!");
        return;
    }

    if (exists(path)) {
        return;
    }

#ifdef NMS_CC_MSVC
    const auto ret = ::mkdir(cpath.data());
#else
    const auto ret = ::mkdir(cpath.data(), 0755);
#endif

    if (ret != 0) {
        log::error("nms.io.mkdir: failed");
        NMS_THROW(ESystem());
    }
}

NMS_API void remove(const Path& path) {
    auto cpath = path.cstr();
    ::remove(cpath);
}

NMS_API bool exists(const Path& path) {
    const StrView cpath = path;
    if (cpath.count()==0) {
        log::error("nms.io.exists: null path!");
        return false;
    }

    const auto ret = ::access(cpath.data(), 0);
    return ret == 0;
}

NMS_API void rename(const Path& oldpath, const Path& newpath) {
    auto old_cpath = oldpath.cstr();
    auto new_cpath = newpath.cstr();
    ::rename(old_cpath, new_cpath);
}
}
