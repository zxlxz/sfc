#include "config.h"
#include "ustd/io.h"
#include "ustd/console.h"
#include "ustd/test.h"

using namespace ustd::io;

namespace ustd::console
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

fn set_fg(int fid, Color c) -> bool {
    if (!is_term(fid)) {
        return false;
    }

    if (c == Color::RST) {
        c = Color::WHT;
    }

    const char cmd[] = { '\033', '[', '3', char(c), 'm' };
    let ret = ::write(fid, cmd, sizeof(cmd));
    return usize(ret) == sizeof(cmd) - 1;
}

fn set_bg(int fid, Color c) -> bool {
    if (!is_term(fid)) {
        return false;
    }

    if (c == Color::RST) {
        c = Color::BLK;
    }

    const char cmd[] = { '\033', '[', '3', char(c), 'm' };
    let ret = ::write(STDOUT_FILENO, cmd, sizeof(cmd));
    return usize(ret) == sizeof(cmd) - 1;
}

[unittest(set_fg)]
{
    mut handle = io::stdout().lock();

    handle.write("    ");
    handle.set_fg(console::BLK); handle.write("[BLK]"); handle.set_fg(console::RST);
    handle.set_fg(console::RED); handle.write("[RED]"); handle.set_fg(console::RST);
    handle.set_fg(console::GRN); handle.write("[GRN]"); handle.set_fg(console::RST);
    handle.set_fg(console::YEL); handle.write("[YEL]"); handle.set_fg(console::RST);
    handle.set_fg(console::BLU); handle.write("[BLU]"); handle.set_fg(console::RST);
    handle.set_fg(console::MAG); handle.write("[MAG]"); handle.set_fg(console::RST);
    handle.set_fg(console::CYN); handle.write("[CYN]"); handle.set_fg(console::RST);
    handle.set_fg(console::WHT); handle.write("[WHT]"); handle.set_fg(console::RST);
    handle.writeln();
};

[unittest(set_bg)]
{
    mut handle = io::stdout().lock();
    handle.write("    ");
    handle.set_bg(console::BLK); handle.write("[BLK]"); handle.set_bg(console::RST);
    handle.set_bg(console::RED); handle.write("[RED]"); handle.set_bg(console::RST);
    handle.set_bg(console::GRN); handle.write("[GRN]"); handle.set_bg(console::RST);
    handle.set_bg(console::YEL); handle.write("[YEL]"); handle.set_bg(console::RST);
    handle.set_bg(console::BLU); handle.write("[BLU]"); handle.set_bg(console::RST);
    handle.set_bg(console::MAG); handle.write("[MAG]"); handle.set_bg(console::RST);
    handle.set_bg(console::CYN); handle.write("[CYN]"); handle.set_bg(console::RST);
    handle.set_bg(console::WHT); handle.write("[WHT]"); handle.set_bg(console::RST);
    handle.writeln();
};

[unittest(Color)]{
    mut handle = io::stdout().lock();
    
    for (u8 fid = console::BLK; fid <= console::WHT; ++fid) {
        handle.write("    ");
        handle.set_fg(console::Color(fid));
        for (u8 bid = console::BLK; bid <= console::WHT; ++bid) {
            handle.set_bg(console::Color(bid));
            handle.write("|{c}{c}| ", fid, bid);
        }
        handle.set_bg(console::RST);
        handle.writeln();
    }
    handle.set_fg(console::RST);
};

}
