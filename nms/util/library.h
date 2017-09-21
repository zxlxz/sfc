#pragma once

#include <nms/core/type.h>

namespace nms
{

class Library
{
public:
    struct Function
    {
    public:
        constexpr Function(void* address=nullptr) noexcept
            : address_(address)
        {}

        operator bool() const {
            return address_ != nullptr;
        }

        template<class T>
        operator T() const {
            return reinterpret_cast<T>(address_);
        }

    private:
        void*   address_;
    };

    NMS_API Library(const StrView&  path);
    NMS_API ~Library();
    NMS_API Function operator[](StrView name) const;

    operator bool() const {
        return object_ != nullptr;
    }
protected:
    void*   object_;
};

}
