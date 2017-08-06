#include <nms/core.h>
#include <nms/core/string.h>

namespace nms
{

NMS_API String& tlsString() {
    static thread_local String buf;

    static thread_local auto _init = [&] {
        buf.reserve(32768);
        return 0;
    }();
    (void)_init;

    return buf;
}

NMS_API u32 strlen(const char* s) {
    if (s == nullptr) {
        return 0u;
    }
    auto len = u32(::strlen(s));
    return len;
}

NMS_API StrView cstr(const char* s) {
    if (s==nullptr) {
        return {};
    }
    auto len = strlen(s);
    return {s, {len}};
}

/* --- split --- */
static bool contains(char c, StrView str) {
    const auto n = str.count();
    for (u32 i = 0; i < n; ++i) {
        if (str[i] == c) return true;
    }
    return false;
}


NMS_API List<StrView> split(StrView str, StrView delimiters) {
    List<StrView> list;

    while (true) {
        const auto n = str.count();
        if (n == 0) {
            break;
        }

        auto b = 0u;
        while (contains(str[b], delimiters) == true && b < n) ++b;
        auto e = b + 1;
        while (contains(str[e], delimiters) == false && e < n) ++e;
        auto s = str.slice( b, e - 1);
        list.append(s);
        if (e + 1 >= n) break;
        str = str.slice(e + 1, n - 1);
    }

    return list;
}

}
