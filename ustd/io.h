#pragma once

#include <ustd/core.h>
#include <ustd/sync.h>
#include <ustd/console.h>

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

template<typename T=void>
using Result    = result::Result<T, Error>;

class Stdout
{
  public:
    static fn instance() -> Stdout& {
        static Stdout obj;
        return obj;
    }

  public:
    using Color = console::Color;
    fn is_term()        -> bool { return console::is_term(1);   }
    fn set_fg(Color c)  -> bool { return console::set_fg(1, c); }
    fn set_bg(Color c)  -> bool { return console::set_bg(1, c); }

    fn write(str s)     -> Result<usize>;
    fn writeln(str s)   -> Result<usize>;
    fn writeln()        -> Result<usize>;

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
  public:
    class Lock: public sync::Mutex::Guard
    {
        Stdout& _ref;
    public:
        Lock(sync::MutexGuard&& guard, Stdout& ref): sync::Mutex::Guard(as_mov(guard)), _ref(ref)
        {}

        Lock(Lock&& lock)
            : sync::Mutex::Guard(as_mov(lock)), _ref(lock._ref) {
        }

        Lock(const Lock&) = delete;
        void operator=(Lock&) = delete;

    public:
        fn is_term()        -> bool { return _ref.is_term(); }
        fn set_fg(Color c)  -> bool { return _ref.set_fg(c); }
        fn set_bg(Color c)  -> bool { return _ref.set_bg(c); }

        template<class ...U>
        fn write(const U& ...u) -> void {
            _ref.write(u...);
        }

        template<class ...U>
        fn writeln(const U& ...u) -> void {
            _ref.writeln(u...);
        }
    };

    [[nodiscard]] fn lock() -> Lock {
        return Lock(_mutex.lock().unwrap(), *this);
    }

  private:
    sync::Mutex _mutex;

    Stdout() = default;
    Stdout(const Stdout&)          = delete;
    void operator=(const Stdout&)  = delete;
};

inline fn stdout()->Stdout& {
    return Stdout::instance();
}

}

