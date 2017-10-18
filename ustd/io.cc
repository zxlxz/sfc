#include "config.h"
#include "ustd/io.h"
#include "ustd/time.h"
#include "ustd/test.h"

using namespace ustd::console;

namespace ustd::io
{

static fn write(int fid, str s) -> Result<usize> {
    using Result = Result<usize>;

    let ret = ::write(fid, s.data(), s.len());

    if (ret >= 0) {
        return Result::Ok(usize(ret));
    }
    else {
        return Result::Err(Error::Other);
    }
}

static fn writeln(int fid, str s) -> Result<usize> {
    using Result = Result<usize>;

    vec::Vec<char, 4096> line_buf;

    if (is_term(fid)) {
        let  time_dur       = time::Duration::since_proc_start();
        let  time_secs      = time_dur._secs + time_dur._nanos*1e-9;
        let  term_columns   = columns(fid);

        if (term_columns > 0 && time_secs > 0.0) {
            let time_len = snprintf(line_buf._data, line_buf.capacity(), "\033[32m\033[%dG%12.3f\033[0m\033[0G", (term_columns - 12), time_secs);
            line_buf._size = u32(time_len);

            if (s._size + 1 < line_buf.capacity() - line_buf.len()) {
                line_buf.push_str(s);
                line_buf.push_str("\n");
                return write(fid, line_buf);
            }
            else {
                let r1 = write(fid, line_buf);
                let r2 = write(fid, s);
                let r3 = write(fid, "\n");
                return r3.is_ok() ? Result::Ok(r1.unwrap() + r2.unwrap() + r3.unwrap()) : r3;
            }
        }
    }
    else {
        if (s._size + 1 < line_buf.capacity()) {
            line_buf.push_str(s);
            line_buf.push_str("\n");
            return write(fid, line_buf);
        }
        else {
            let r1 = write(fid, s);
            let r2 = write(fid, "\n");
            return r2.is_ok() ? Result::Ok(r1.unwrap() + r2.unwrap()) : r2;
        }
    }

    return Result::Ok(0u);
}

fn Stdout::write(str s)->Result<usize> {
    return io::write(STDOUT_FILENO, s);
}

fn Stdout::writeln(str s)->Result<usize> {
    return io::writeln(STDOUT_FILENO, s);
}

fn Stdout::writeln()->Result<usize> {
    let s = str{};
    return io::writeln(STDOUT_FILENO, s);
}

}

