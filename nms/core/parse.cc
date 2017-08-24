#include <nms/core/parse.h>
#include <nms/core/time.h>

namespace nms
{

template<class T>
static int _parse(StrView str, T& val, const char* fmt) {
    const auto buf = str.data();

#ifdef _MSC_VER
    const auto cnt = str.count();
    const auto ret = _snscanf_s(buf, cnt, fmt, &val);
#else
    const auto ret = sscanf(buf, fmt, &val);
#endif
    return ret > 0;
}

NMS_API int parseImpl(StrView str, StrView fmt, u8&  val) { return _parse(str, val, "%hhu"); }
NMS_API int parseImpl(StrView str, StrView fmt, i8&  val) { return _parse(str, val, "%hhd"); }
NMS_API int parseImpl(StrView str, StrView fmt, u16& val) { return _parse(str, val, "%hu");  }
NMS_API int parseImpl(StrView str, StrView fmt, i16& val) { return _parse(str, val, "%hd");  }
NMS_API int parseImpl(StrView str, StrView fmt, u32& val) { return _parse(str, val, "%u");   }
NMS_API int parseImpl(StrView str, StrView fmt, i32& val) { return _parse(str, val, "%d");   }
NMS_API int parseImpl(StrView str, StrView fmt, u64& val) { return _parse(str, val, "%Lu");  }
NMS_API int parseImpl(StrView str, StrView fmt, i64& val) { return _parse(str, val, "%Ld");  }
NMS_API int parseImpl(StrView str, StrView fmt, f32& val) { return _parse(str, val, "%f");   }
NMS_API int parseImpl(StrView str, StrView fmt, f64& val) { return _parse(str, val, "%lf");  }

}