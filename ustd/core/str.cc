#include "config.h"
#include "ustd/core.h"

namespace ustd
{

fn Slice<const char, 0>::from_raw(const char* s)-> Slice {
    if (s == nullptr) {
        return {};
    }
    let n = ::strlen(s);
    return { s, u32(n) };
}

}