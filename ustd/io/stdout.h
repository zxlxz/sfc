#pragma once

#include <ustd/core.h>
#include <ustd/sync/mutex.h>

namespace ustd::io
{

enum class Error
{
    Success,
    NotFound,
    PermissionDenied,
    ConnectionRefused,
    ConnectionReset,
    ConnectionAborted,
    NotConnected,
    AddrInUse,
    AddrNotAvailable,
    BrokenPipe,
    AlreadyExists,
    WouldBlock,
    InvalidInput,
    InvalidData,
    TimedOut,
    WriteZero,
    Interrupted,
    UnexpectedEof,
    Other,
};

enum class TermColor : i8
{
    RST = 0,
    BLK = '0',
    RED = '1',
    GRN = '2',
    YEL = '3',
    BLU = '4',
    MAG = '5',
    CYN = '6',
    WHT = '7'
};

template<typename T = void>
using Result = result::Result<T, Error>;

class Stdout;
class StdoutLock;

class Stdout
{
  public:
    friend class StdoutLock;
    static fn instance()->Stdout&;

    fn is_term()            -> bool;
    fn set_fg(TermColor c)  -> void;
    fn set_bg(TermColor c)  -> void;

    fn write(str s)         -> Result<usize>;
    fn writeln(str s)       -> Result<usize>;
    fn writeln()            -> Result<usize>;

    template<class ...U>
    fn write(str fmt, const U& ...u) -> Result<usize> {
        vec::Vec<char, 4096> buf;
        sformat(buf, fmt, u...);
        return this->write(buf);
    }

    template<class ...U>
    fn writeln(str fmt, const U& ...u) -> Result<usize> {
        vec::Vec<char, 4096> buf;
        sformat(buf, fmt, u...);
        return this->writeln(buf);
    }

    [[nodiscard]] fn lock() -> StdoutLock;

  private:
    sync::Mutex<int> _mutex;

    Stdout() : _mutex{0}
    {}

    Stdout(const Stdout&) = delete;
    void operator=(const Stdout&) = delete;
};

class StdoutLock : sync::MutexGuard<>
{
  public:
    StdoutLock(StdoutLock&& other)
        : sync::MutexGuard<>(as_mov(other)), _ref(other._ref)
    {}

    template<class ...U>
    fn write(const U& ...u) -> void {
        _ref.write(u...);
    }

    template<class ...U>
    fn writeln(const U& ...u) -> void {
        _ref.writeln(u...);
    }

    fn is_term()            -> bool { return _ref.is_term(); }
    fn set_fg(TermColor c)  -> void { return _ref.set_fg(c); }
    fn set_bg(TermColor c)  -> void { return _ref.set_bg(c); }

  private:
    Stdout& _ref;

    friend class Stdout;
    StdoutLock(sync::MutexGuard<>&& guard, Stdout& ref)
        : sync::MutexGuard<>(as_mov(guard)), _ref(ref)
    {}

    StdoutLock(const StdoutLock&) = delete;
    void operator=(StdoutLock&) = delete;
};

inline fn Stdout::instance() -> Stdout& {
    static Stdout obj;
    return obj;
}

inline [[nodiscard]] fn Stdout::lock()->StdoutLock {
    return StdoutLock(_mutex.lock().unwrap(), *this);
}


inline fn stdout()-> Stdout& {
    return Stdout::instance();
}

}

