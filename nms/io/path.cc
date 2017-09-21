#include <nms/test.h>
#include <nms/util/system.h>
#include <nms/io/path.h>

namespace nms::io
{

extern "C" {
    void* GetModuleHandleA(const char* lpModuleName);
    u32   GetModuleFileNameA(void* hModule, char lpFileName[], u32 nSize);
}

static StrView _get_app_dir() {

    // /opt/app/bin/ddd.exe
    //         ^   ^       ^
    //         |   |     exe_len
    //         | bin_pos
    //       app_pos

    static U8String<256> exe_path;
    u32 exe_len = 0;

#if defined(NMS_OS_WINDOWS)
    const auto mod = GetModuleHandleA(nullptr);
    exe_len   = GetModuleFileNameA(mod, exe_path.data(), exe_path.capacity());
    exe_path.resize(exe_len);
    exe_path.replace('\\', '/');

#elif defined(NMS_OS_MACOS)
    exe_len = exe_path.capicity();
    _NSGetExecutablePath(exe_path.data(), &exe_len);
    bin_path.resize(exe_len);

#elif defined(NMS_OS_UNIX)
    auto ret  = ::readlink("/proc/self/exe", exe_path.data(), exe_path.capacity());
    if (ret == -1) {
        return {};
    }
    exe_len = u32(ret);
#else
    #   errno "unknow system"
#endif

    if (exe_len == 0) {
        return {};
    }

    u32 bin_pos = exe_len - 1;
    while (bin_pos > 0 && exe_path[bin_pos] != '/') {
        --bin_pos;
    }

    if (bin_pos == 0) {
        return {};
    }

    const auto bin_path = exe_path.slice(0u, bin_pos);

    u32 app_pos = bin_pos - 1;
    while (app_pos > 0 && bin_path[app_pos] != '/') {
        --app_pos;
    }

    if (app_pos == 0) {
        return {};
    }

    const auto app_path = bin_path.slice(0u, app_pos);
    return app_path;
}

#ifdef NMS_OS_WINDOWS
static StrView _get_home_dir() {
    static U8String<256> home_dir;

#if defined(NMS_OS_UNIX)
    home_dir = system::getenv("HOME");
#elif  defined(NMS_OS_WINDOWS)
    home_dir = system::getenv("USERPROFILE");
    home_dir.replace('\\', '/');
#else
#   error "unexpected system"
#endif

    home_dir += '/';
    return home_dir;
}
#endif

NMS_API Path& Path::operator/=(const StrView& rhs) {
    const auto str_len = str_.count();

    if (str_len == 0) {
        str_ = rhs;
    }
    else {
        if (str_[str_len-1] != '/') {
            str_ += "/";
        }
        str_ += rhs;
    }
    return *this;
}

NMS_API Path& Path::operator+=(const StrView& rhs) {
    str_ += rhs;
    return *this;
}

NMS_API StrView Path::ext() const {
    const auto  cnt = str_.count();
    auto        idx = cnt - 1;
    while (idx > 0 && str_[idx] != '.') {
        --idx;
    }

    const auto ret = (idx != 0) ? str_.slice(idx, cnt - 1) : StrView{};
    return  ret;
}

NMS_API StrView Path::base() const {
    const auto  cnt = str_.count();
    auto        idx = cnt - 1;
    while (idx > 0 && str_[idx] != '.') {
        --idx;
    }

    const auto ret = idx != 0 ? str_.slice(0u, idx - 1) : StrView{ str_ };
    return ret;
}

NMS_API StrView Path::path() const {
    static const auto app_dir  = _get_app_dir();

#ifdef NMS_OS_WINDOWS
    static const auto home_dir = _get_home_dir();
#endif

    static thread_local U8String<1024>  full_path;
    full_path.resize(0);

    // check if empty
    if (str_.count() == 0) {
        return {};
    }

    // if symbols
    if (str_.count() >= 2 && str_[1] == '/') {
        switch(str_[0]) {
            case '#':
                full_path += app_dir;
                full_path += str_.slice(2, -1);
                break;
#ifdef NMS_OS_WINDOWS
            case '~':
                full_path += home_dir;
                full_path += str_.slice(2, -1);
                break;
#endif
            default:
                break;
        }
    }
    // if not null terminal
    else if (str_[str_.count()] != '\0') {
        full_path += str_;
    }
    else {
        return str_;
    }

    // out-of-range
    if (full_path.count() == full_path.capacity()) {
        return {};
    }

    // add zero
    full_path[full_path.count()] = '\0';
    return full_path;
}

NMS_API Path cwd() {
    U8String<1024> buff;

#ifdef NMS_OS_WINDOWS
    const auto ret = ::getcwd(buff.data(), i32(buff.capacity()));
#else
    const auto ret = ::getcwd(buff.data(), size_t(buff.capacity()));
#endif
    if (ret == nullptr) {
        return {};
    }

    const auto len = strlen(buff.data());
    buff.resize(len);

#ifdef NMS_OS_WINDOWS
    buff.replace('\\', '/');
#endif

    return Path(buff);
}

NMS_API void chdir(const Path& path) {
    const StrView full_path = path.path();

    if (full_path.count()==0) {
        log::error("nms.io.chdir: null path!!");
        NMS_THROW(ESystem(0));
    }

    const auto ret = ::chdir(full_path.data());

    if (ret != 0) {
        log::error("nms.io.chdir: cannot chdir to '{}'", full_path);

        if (!exists(path)) {
            log::error("nms.io.chdir: '{}' not exists", full_path);
        }
        NMS_THROW(ESystem());
    }
}

NMS_API void mkdir(const Path& path) {
    const auto full_path = path.path();

    if (full_path.count()==0) {
        log::error("nms.io.chdir: null path!!");
        return;
    }

    if (exists(path)) {
        return;
    }

#ifdef NMS_CC_MSVC
    const auto ret = ::mkdir(full_path.data());
#else
    const auto ret = ::mkdir(full_path.data(), 0755);
#endif

    if (ret != 0) {
        log::error("nms.io.mkdir: cannot mkdir '{}'", full_path);
        NMS_THROW(ESystem());
    }
}

NMS_API void remove(const Path& path) {
    const auto full_path = path.path();

    if (full_path.count() == 0) {
        log::error("nms.io.remove: null path");
        return;
    }
    const auto ret = ::remove(full_path.data());
    if (ret != 0) {
        log::error("nms.io.remove: cannot remove '{}'", full_path);
    }
}

NMS_API bool exists(const Path& path) {
    const auto full_path = path.path();
    if (full_path.count()==0) {
        log::error("nms.io.exists: null path!");
        return false;
    }

    const auto ret = ::access(full_path.data(), 0);
    return ret == 0;
}

NMS_API void rename(const Path& oldpath, const Path& newpath) {
    const auto old_cpath = oldpath.path();
    const auto new_cpath = newpath.path();

    if (old_cpath.count() == 0 || new_cpath.count() == 0) {
        return;
    }

    const auto ret = ::rename(old_cpath.data(), new_cpath.data());
    if (ret != 0) {
        log::error("nms.io.rename: cannot rename '{}' -> '{}'", old_cpath, new_cpath);
    }
}
}
