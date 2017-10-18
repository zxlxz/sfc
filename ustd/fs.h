#pragma once

#include <ustd/core.h>
#include <ustd/io.h>

namespace ustd::fs
{

using io::Error;
using io::Result;


class File
{
    int _handle;

    File() = default;

  public:
    File(File&& other) noexcept : _handle(other._handle) {
        other._handle = -1;
    }

    ~File() noexcept {
        if (_handle != -1) {
            _close(_handle);
            _handle = -1;
        }
    }

    File(const File&) = delete;
    void operator=(const File&) = delete;

    /* constructor */
    template<typename P> static fn open  (P path) noexcept;
    template<typename P> static fn create(P path) noexcept;

    /* read */
    fn read(Slice<u8> buf) noexcept -> Result<usize>;

    /* write*/
    fn write(Slice<const u8> buf) noexcept -> Result<usize>;

  private:
    static fn _close (int  handle)           noexcept -> void;
    static fn _open  (int* handle, str path) noexcept -> Error;
    static fn _create(int* handle, str path) noexcept -> Error;
};

template<typename P> fn File::open  (P path) noexcept {   }
template<typename P> fn File::create(P path) noexcept {   }

}


