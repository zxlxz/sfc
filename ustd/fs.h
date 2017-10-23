#pragma once

#include <ustd/core.h>
#include <ustd/io.h>

namespace ustd::fs
{

using io::Error;
using io::Result;

class File: public $class
{
    int _handle;

    File(int handle) : _handle(handle)
    {}

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
    fn read(View<u8> buf) noexcept -> Result<usize>;

    /* write*/
    fn write(View<const u8> buf) noexcept -> Result<usize>;

  private:
    static fn _close (int  handle)           noexcept -> void;
    static fn _open  (int* handle, str path) noexcept -> Error;
    static fn _create(int* handle, str path) noexcept -> Error;
};

template<typename P>
fn File::open(P path) noexcept { 
    using Result = Result<File>;

    let fid = int(0);
    let eid = File::_open(&fid, path);
    return eid == Error::Success ? Result::Ok(File(fid)) : Result::Err(eid);
}

template<typename P>
fn File::create(P path) noexcept {
    using Result = Result<File>;

    let fid = int(0);
    let eid = File::_create(&fid, path);
    return eid == Error::Success ? Result::Ok(File(fid)) : Result::Err(eid);
}

}


