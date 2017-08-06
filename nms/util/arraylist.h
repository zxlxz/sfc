#pragma once

#include <nms/core/type.h>
#include <nms/core/view.h>
#include <nms/core/memory.h>

namespace nms
{

template<class T, u32 BookSize, u32 PageSize>
class ArrayList
    : public INocopyable
{
public:
    using Type = T;
    constexpr static u32 $BookSize  = BookSize;     // count max pages in book
    constexpr static u32 $PageSize  = PageSize;     // count max items in page

    ArrayList()
        : book_(nullptr), count_(0)
    {}

    ~ArrayList() {
        if (book_ == nullptr) {
            return;
        }

        const auto page_count = getPageCount();
        for (u32 pid = 0; pid < page_count; ++pid) {
            auto page = book_[pid];
            mdel(page);
        }
        mdel(book_);
    }

    const Type& operator[](u32 idx) const {
        const auto bid  = idx / $BookSize;
        const auto pid  = idx % $BookSize;

        const auto& page = book_[bid];
        const auto& data = page[pid];
        return data;
    }

    Type& operator[](u32 idx) {
        const ArrayList& self = *this;
        const auto&      data = self[idx];
        return const_cast<Type&>(data);
    }

    u32 count() const noexcept {
        return count_;
    }

    u32 getPageCount() const {
        const auto value = (count_ + $PageSize - 1) / $PageSize;
        return value;
    }

    void push(const Type& value) {
        const auto idx = count_;
        const auto bid = idx / $PageSize;
        const auto pid = idx % $PageSize;

        // check if out of range?
        if (bid >= $BookSize) {
            throw EOutOfRange{};
        }

        // check book
        if (book_ == nullptr) {
            book_ = mnew<T*>($BookSize);
            mzero(book_, $BookSize);
        }

        // check page
        if (book_[bid] == nullptr) {
            book_[bid] = mnew<Type>($PageSize);
        }

        // add value
        book_[bid][pid] = value;

        // add count
        ++count_;
    }

    void getData(Type* buff) const {
        const auto count        = count_;
        const auto page_count   = getPageCount();
        const auto last_count   = count - (page_count - 1)*$PageSize;

        for (u32 bid = 0; bid < page_count; ++bid) {
            const auto src = book_[bid];
            const auto dst = buff + bid*$PageSize;
            const auto len = (bid != page_count - 1) ? $PageSize : last_count;
            mcpy(dst, src, len);
        }
    }

    void setData(Type* buff) {
        const auto count        = count_;
        const auto page_count   = (count + $PageSize  - 1) / $PageSize;
        const auto last_count   = count - (page_count - 1) * $PageSize;

        for (u32 pid = 0; pid < page_count; ++pid) {
            const auto dst = book_[pid];
            const auto src = buff + pid*$PageSize;
            const auto len = (pid != page_count - 1) ? $PageSize : last_count;
            mcpy(dst, src, len);
        }
    }

#pragma region save/load
    template<class OS>
    friend OS& operator<<(OS& os, const ArrayList& list) {
        const auto info       = View<T,1>::typeinfo();
        const auto count      = list.count_;
        const auto page_count = list.getPageCount();
        const auto last_count = count - (page_count - 1) * $PageSize;

        os.write(&info, 1);
        os.write(&count, 1);
        for (u32 bid = 0; bid < page_count; ++bid) {
            const auto dat = list.book_[bid];
            const auto len = (bid != page_count - 1) ? $PageSize : last_count;
            os.write(dat, len);
        }
        return os;
    }
#pragma endregion

protected:
    T**     book_   = nullptr;
    u32     count_  = 0;
};

}
