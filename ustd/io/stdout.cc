#include "config.h"

namespace ustd::io
{

namespace term
{

fn is_term(int fid) -> bool {
    let ret = ::isatty(fid);
    return ret != 0;
}

fn columns(int fid)-> u32 {
    if (!is_term(fid)) {
        return false;
    }

    struct winsize w;
    ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return u32(w.ws_col);
}

fn set_fg(int fid, TermColor c) -> bool {
    if (!is_term(fid)) {
        return false;
    }

    if (c == TermColor::RST) {
        c = TermColor::WHT;
    }

    const char cmd[] = { '\033', '[', '3', char(c), 'm' };
    let ret = ::write(fid, cmd, sizeof(cmd));
    return usize(ret) == sizeof(cmd) - 1;
}

fn set_bg(int fid, TermColor c) -> bool {
    if (!is_term(fid)) {
        return false;
    }

    if (c == TermColor::RST) {
        c = TermColor::BLK;
    }

    const char cmd[] = { '\033', '[', '3', char(c), 'm' };
    let ret = ::write(STDOUT_FILENO, cmd, sizeof(cmd));
    return usize(ret) == sizeof(cmd) - 1;
}

}

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

    if (term::is_term(fid)) {
        let  time_dur = time::Duration::since_proc_start();
        let  time_secs = time_dur._secs + time_dur._nanos*1e-9;
        let  term_columns = term::columns(fid);

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

fn Stdout::is_term() -> bool {
    return term::is_term(STDOUT_FILENO);
}

fn Stdout::set_fg(TermColor c)  -> void {
    term::set_fg(STDOUT_FILENO, c);
}

fn Stdout::set_bg(TermColor c)  -> void {
    term::set_bg(STDOUT_FILENO, c);
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

[unittest(set_fg)]
{
    mut handle = io::stdout().lock();

    handle.write("    ");
    handle.set_fg(TermColor::BLK); handle.write("[BLK]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::RED); handle.write("[RED]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::GRN); handle.write("[GRN]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::YEL); handle.write("[YEL]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::BLU); handle.write("[BLU]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::MAG); handle.write("[MAG]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::CYN); handle.write("[CYN]"); handle.set_fg(TermColor::RST);
    handle.set_fg(TermColor::WHT); handle.write("[WHT]"); handle.set_fg(TermColor::RST);
    handle.writeln();
};

[unittest(set_bg)]
{
    mut handle = io::stdout().lock();
    handle.write("    ");
    handle.set_bg(TermColor::BLK); handle.write("[BLK]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::RED); handle.write("[RED]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::GRN); handle.write("[GRN]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::YEL); handle.write("[YEL]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::BLU); handle.write("[BLU]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::MAG); handle.write("[MAG]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::CYN); handle.write("[CYN]"); handle.set_bg(TermColor::RST);
    handle.set_bg(TermColor::WHT); handle.write("[WHT]"); handle.set_bg(TermColor::RST);
    handle.writeln();
};

[unittest(Color)]{
    mut handle = io::stdout().lock();

    for (u8 fid = u8(TermColor::BLK); fid <= u8(TermColor::WHT); ++fid) {
        handle.write("    ");
        handle.set_fg(TermColor(fid));
        for (u8 bid = u8(TermColor::BLK); bid <= u8(TermColor::WHT); ++bid) {
            handle.set_bg(TermColor(bid));
            handle.write("|{c}{c}| ", fid, bid);
        }
        handle.set_bg(TermColor::RST);
        handle.writeln();
    }
    handle.set_fg(TermColor::RST);
};

}

