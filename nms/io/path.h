#pragma once

#include <nms/core.h>

namespace nms::io
{

class ENotExists: public IException
{};

class Path final
{
public:
    Path()
        : str_{}
    {}

    Path(const StrView& s) {
        init(s);
    }

    template<u32 N>
    Path(const char(&s)[N])
        : Path(StrView{ s })
    {}

    NMS_API Path  operator/ (const Path& rhs) const;
    NMS_API Path& operator/=(const Path& rhs);
    NMS_API Path& operator+=(const Path& rhs);

    NMS_API StrView ext() const;
    NMS_API StrView base() const;

    StrView str() const {
        return str_;
    }

    operator StrView() const {
        return str_;
    }

    operator const String&() const {
        return str_;
    }

    const char* cstr() const {
        return str().data();
    }

private:
    String str_;

    NMS_API void init(StrView s);
};

NMS_API const Path& prefix();
NMS_API Path  cwd();

NMS_API void  chdir(const Path& path);
NMS_API void  mkdir(const Path&);
NMS_API bool  exists(const Path& path);
NMS_API u64   fsize(const Path& path);

inline void formatImpl(String& buf, const Path& value, StrView fmt) {
    StrView strval = value;
    formatImpl(buf, strval, fmt);
}

}
