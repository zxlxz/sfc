#pragma once

#include <nms/core/type.h>
#include <nms/core/view.h>
#include <nms/core/memory.h>

namespace nms
{

template<class T, u32 Ibooksize, u32 Ipagesize>
class ArrayList
{
public:
    using Tdata = T;
    constexpr static u32 $book_size  = Ibooksize;     // count max pages in book
    constexpr static u32 $page_size  = Ipagesize;     // count max items in page

#pragma region constructor

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

#pragma endregion

#pragma region properties
    u32 count() const noexcept {
        return count_;
    }

    u32 getPageCount() const {
        const auto value = (count_ + $page_size - 1) / $page_size;
        return value;
    }

    static auto info() {
        return mk_viewinfo<Tdata, 0>();
    }
#pragma endregion

#pragma region methods
    const Tdata& operator[](u32 idx) const {
        const auto bid  = idx / $book_size;
        const auto pid  = idx % $book_size;

        const auto& page = book_[bid];
        const auto& data = page[pid];
        return data;
    }

    Tdata& operator[](u32 idx) {
        const ArrayList& self = *this;
        const auto&      data = self[idx];
        return const_cast<Tdata&>(data);
    }

    void push(const Tdata& value) {
        const auto idx = count_;
        const auto bid = idx / $page_size;
        const auto pid = idx % $page_size;

        // check if out of range?
        if (bid >= $book_size) {
            NMS_THROW(EOutOfRange<u32>{0, $book_size, bid});
        }

        // check book
        if (book_ == nullptr) {
            book_ = mnew<T*>($book_size);
            mzero(book_, $book_size);
        }

        // check page
        if (book_[bid] == nullptr) {
            book_[bid] = mnew<Tdata>($page_size);
        }

        // add value
        book_[bid][pid] = value;

        // add count
        ++count_;
    }

    void getData(Tdata* buff) const {
        const auto count        = count_;
        const auto page_count   = getPageCount();
        const auto last_count   = count - (page_count - 1)*$page_size;

        for (u32 bid = 0; bid < page_count; ++bid) {
            const auto src = book_[bid];
            const auto dst = buff + bid*$page_size;
            const auto len = (bid != page_count - 1) ? $page_size : last_count;
            mcpy(dst, src, len);
        }
    }

    void setData(Tdata* buff) {
        const auto count        = count_;
        const auto page_count   = (count + $page_size  - 1) / $page_size;
        const auto last_count   = count - (page_count - 1) * $page_size;

        for (u32 pid = 0; pid < page_count; ++pid) {
            const auto dst = book_[pid];
            const auto src = buff + pid*$page_size;
            const auto len = (pid != page_count - 1) ? $page_size : last_count;
            mcpy(dst, src, len);
        }
    }
#pragma endregion

#pragma region save/load
    void save(io::File& file) const {
        return this->saveFile(file);
    }

    void save(const io::Path& path) const {
        return this->savePath<io::File>(path);
    }
#pragma endregion

protected:
    T**     book_   = nullptr;
    u32     count_  = 0;

private:
    template<class File>
    void saveFile(File& file) const {
        const auto info       = this->info();
        const auto count      = this->count();
        const auto page_count = this->getPageCount();
        const auto last_count = count - (page_count - 1) * $page_size;

        file.write(&info, 1);
        file.write(&count, 1);
        for (u32 bid = 0; bid < page_count; ++bid) {
            const auto dat = this->book_[bid];
            const auto len = (bid != page_count - 1) ? $page_size : last_count;
            file.write(dat, len);
        }
    }

    template<class File, class Path>
    void savePath(const Path& path) const {
        File file(path, File::Write);
        saveFile(file);
    }
};

}
