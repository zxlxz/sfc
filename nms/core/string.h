#pragma once

#include <nms/core/base.h>
#include <nms/core/view.h>
#include <nms/core/list.h>

namespace nms
{

class String;

NMS_API u32     strlen(const char* s);
NMS_API StrView cstr(const char* s);

/* string */
class String final
    : public List<char, 32>
{
    using base = nms::List<char, 32>;

public:
#pragma region constructor
    /*! construct a empty string */
    constexpr String() noexcept 
    {}

    /* destructor */
    ~String() = default;

    /* constructor: redirect to List */
    template<class ...U>
    __forceinline String(U&& ...args)
        : base(fwd<U>(args)...) 
    {}

#ifdef _M_CEE
    __forceinline String(System::String^ str)
    {
        const auto cnt = u32(str->Length);
        resize(cnt);
        const auto dat = data();
        for (auto i = 0u; i < cnt; ++i) {
            dat[i] = char(str[i]);
        }
    }
#endif
#pragma endregion

    /*!
     * resize the string
     */
    String& resize(u32 n) {
        reserve(n);
        size_[0] = n;
        return *this;
    }

    /*!
     * returns a cstring (null terminal)
     */
    const char* cstr() const {
        if (base::count() == 0) {
            return "";
        }

        auto& self = const_cast<String&>(*this);
        if (at(count()-1) != '\0') {
            const auto oldlen = self.count();
            const auto newlen = oldlen+1;
            self.reserve(newlen);
            self.data()[oldlen] = '\0';
        }
        return data();
    }

    /*!
     * find the first index of c 
     */
    u32 find(char c) const {
        const auto n = count();
        for (u32 i = 0; i < n; ++i) {
            if (at(i) == c) return i;
        }
        return n;
    }

    auto& operator+=(char c) {
        base::append(c);
        return *this;
    }

    auto& operator+=(StrView s) {
        base::appends(s.data(), s.count());
        return *this;
    }
};

NMS_API String& tlsString();

/**
 * concatenates two strings
 */
inline String operator+(StrView a, StrView b) {
    String c;
    c.reserve(a.count() + b.count() +1);
    c += a;
    c += b;
    return c;
}

/* split a string into pieces */
NMS_API List<StrView> split(StrView str, StrView delimiters);

}
