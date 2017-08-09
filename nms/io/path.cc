#include <nms/config.h>
#include <nms/io/path.h>
#include <nms/io/log.h>

namespace nms::io
{

extern "C" {
    void* GetModuleHandleA(const char* lpModuleName);
    u32   GetModuleFileNameA(void* hModule, char lpFileName[], u32 nSize);
}

struct XPath
{
public:
    XPath(StrView str) {
        if (is_delimiter(str[0])) {
            dir_[count_++] = StrView();
            str = str.slice(1u, str.count()-1);
        }

        while (true) {
            const auto n = str.count();
            if (n == 0) {
                break;
            }

            auto b = 0u;
            while (is_delimiter(str[b]) == true && b < n) ++b;
            auto e = b + 1;
            while (is_delimiter(str[e]) == false && e < n) ++e;
            auto s = str.slice(b, e - 1);

            if (s.count() == 0) {
                break;
            }

            if (s == ".") {
            }
            if (s == ".." && count_ > 0) {
                --count_;
            }
            else {
                dir_[count_] = s;
                ++count_;
            }

            if (e + 1 >= n) break;
            str = str.slice(e + 1, n - 1);
        }
    }

    String toPath() {
        if (count_ == 0) {
            return String();
        }

        auto len = 0u;
        for (u32 i = 0; i < count_; ++i) {
            len += dir_[i].count();
        }

        String tmp;
        tmp.reserve(len + (count_ - 1));
        for (u32 i = 0; i < count_; ++i) {
            tmp += dir_[i];
            if (i != count_ - 1) {
                tmp += "/";
            }
        }
        tmp.cstr();
        return tmp;
    }

protected:
    u32     count_  = 0;
    StrView dir_[64];

    bool is_delimiter(char c) const {
        const auto stat = c == '/' || c == '\\';
        return stat;
    }
};

NMS_API void Path::init(StrView s) {
    XPath xpath(s);
    str_ = move(xpath.toPath());

    if (str_.count() > 2 && str_[0] == '#' && str_[1] == '/') {
        auto& prefix_path = prefix();
        String full_path{StrView{prefix_path}};
        auto sub_path = str_.slice(1, -1);
        full_path += sub_path;
        nms::swap(str_, full_path);
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

NMS_API const Path& prefix() {

    static String exe_path = [] {
        static char exe_path_buf[512];
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
        return cstr(exe_path_buf);
    }();

    static Path dir(exe_path+"/../..");
    return dir;
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
    if (cpath.count()!=0) {
        log::error("nms.io.chdir: null path!!");
        throw ESystem(0);
    }
    const auto ret = ::chdir(cpath.data());
    
    if (ret != 0) {
        log::error("nms.io.chdir: failed");
        throw ESystem(errno);
    }

}

NMS_API void mkdir(const Path& path) {
    const StrView cpath = path;

    if (cpath.count()!=0) {
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
        throw ESystem(errno);
    }
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

NMS_API u64 fsize(const Path& path) {
    auto cpath = path.cstr();
#ifdef NMS_OS_WINDOWS
    struct _stat64 st;
    ::_stat64(cpath, &st);
#else
    struct stat st;
    ::stat(path.cstr(), &st);
#endif
    return st.st_size;
}

}
