#include "config.h"

namespace ustd::fs
{

namespace impl
{
fn _open(str path, int oflag) noexcept -> int {
    static char buf[2048];
    if (path.len() >= sizeof(buf)) {
        return -1;
    }
    ::strncpy(buf, path.data(), path.len());

#ifdef _WIN32
    let pmode = _S_IWRITE;
#else
    let pmode = 0644;   // user:wr, group:r, other:r
#endif
    let fd = ::open(buf, oflag, pmode);
    return fd;
}
}

fn File::_open(int* handle, str path) noexcept-> Error {
    *handle = impl::_open(path, O_RDONLY);
    if (*handle == -1) {
        let eid = errno;
        switch (eid) {
        case EACCES:
            return Error::PermissionDenied;
        case ENOENT:
            return Error::NotFound;
        default:
            return Error::Other;
        }
    }
    return Error::Success;
}

fn File::_create(int* handle, str path) noexcept-> Error {
    *handle = impl::_open(path, O_WRONLY|O_CREAT);
    if (*handle == -1) {
        let eid = errno;
        switch (eid) {
        case EACCES:
            return Error::PermissionDenied;
        case EEXIST:
            return Error::AlreadyExists;
        default:
            return Error::Other;
        }
    }
    return Error::Success;
}

fn File::_close(int handle) noexcept -> void {
    ::close(handle);
}

fn File::read(View<u8> buf) noexcept ->Result<usize> {
    using Result = Result<usize>;

    let ret = ::read(_handle, buf.data(), buf.capacity());
    if (ret == 0) {
        return Result::Err(Error::UnexpectedEof);
    }
    else if (ret < 0) {
        let eid = errno;

        switch (eid) {
        case EINVAL:
        case EBADF:
            return Result::Err(Error::InvalidInput);
        default:
            return Result::Err(Error::InvalidInput); 
        }
    }
    return Result::Ok(usize(ret));
}

fn File::write(View<const u8> buf) noexcept ->Result<usize> {
    using Result = Result<usize>;

    let ret = ::write(_handle, buf.data(), buf.len());

    if (ret == 0) {
        return Result::Err(Error::UnexpectedEof);
    }
    else if (ret < 0) {
        let eid = errno;

        switch (eid) {
        case EINVAL:
        case EBADF:
            return Result::Err(Error::InvalidInput);
        default:
            return Result::Err(Error::InvalidInput);
        }
    }
    return Result::Ok(usize(ret));
}

}