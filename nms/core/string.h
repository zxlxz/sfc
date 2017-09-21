#pragma once

#include <nms/core/list.h>

namespace nms
{

template<u32 Icapacity=0 > using U8String   = List<char,     Icapacity>;
template<u32 Icapacity=0 > using U16String  = List<char16_t, Icapacity>;
template<u32 Icapacity=0 > using Tu32String = List<char32_t, Icapacity>;

template<u32 Icapacity=0 > using String     = U8String<Icapacity>;

NMS_API u32 strlen(const char* s);

inline StrView mkStrView(const char* s) {
    return {s, strlen(s)};
}

/* split a TString into pieces */
NMS_API IList<StrView>& split(IList<StrView>& strs, const StrView& str, const StrView& delimiters);

}
