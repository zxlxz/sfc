#pragma once

#include <nms/core.h>

namespace nms::thread
{
class Mutex;
}

namespace nms::io::console
{

NMS_API void writes(const StrView texts[], u32 n);

inline void  write(StrView text) {
    StrView texts[] = { text };
    writes(texts, 1);
}

inline void  writeln(StrView text) {
    StrView texts[] = { text, "\n" };
    writes(texts, 2);
}

template<class T, class ...U>
void write(StrView fmt, const T& t, const U& ...u) {
    auto& buf = tlsString();
    buf.clear();
    sformat(buf, fmt, t, u...);
    write(buf);
}

template<class T, class ...U>
void writeln(StrView fmt, const T& t, const U& ...u) {
    auto& buf = tlsString();
    buf.resize(0);
    sformat(buf, fmt, t, u...);
    writeln(buf);
}

}
