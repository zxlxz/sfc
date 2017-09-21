#include <nms/test.h>
#include <nms/io/file.h>
#include <nms/util/stackinfo.h>

namespace nms::io::log
{

using namespace nms::thread;

Level   gLevel = Level::None;

NMS_API Level getLevel() {
    return gLevel;
}

NMS_API void setLevel(Level level) {
    gLevel = level;
}

NMS_API TxtFile& gLogFile() {
    static TxtFile file;
    return file;
}

NMS_API void setLogPath(const Path& path) {
    gLogFile().open(path, File::Write);
}

static StrView getConsoleColor(Level type) {
    switch (type) {
        case Level::None:
            return console::$rst;

        case Level::Debug:
            return "";
        case Level::Info:
            return console::$fg_grn;

        case Level::Warn:
            return console::$fg_yel;

        case Level::Alert:
            return console::$bg_grn;

        case Level::Error:
            return console::$fg_red;

        case Level::Fatal:
            return console::$bg_red;

        default:
            return "";
    }
}

NMS_API IString& _gStrBuff() {
    static thread_local U8String<128*1024> str;   // 128KB
    return str;
}

NMS_API void _message(Level level, IString& msg) {
    if (level < gLevel) {
        return;
    }

    msg += '\n';

    // current process time
    const auto time = clock();
    const auto name = mkEnum(level).name();

    // 1. terminal
    {
        const auto color    =  getConsoleColor(level);

        char head[64];
        const auto head_len = snprintf(head, sizeof(head), "%s[%s] %6.3f%s ", color.data(), name.data(), time, console::$rst);

        if (head_len > 0 && u32(head_len) <= sizeof(head) ) {
            const auto offset   = u32(sizeof(head)) - u32(head_len);
            mcpy(msg.data()+offset, head, head_len);

            StrView out{msg.data()+offset, msg.count()-offset};
            console::write(out);
        }

        if (level >= Level::Fatal) {
            StackInfo stack_info;
            console::writeln("{}", stack_info);
        }
    }

    // 2. file
    static auto& log_file = gLogFile();

    if (log_file) {
        char head[64];
        const auto head_len = snprintf(head, sizeof(head), "[%s] %6.3f ", name.data(), time);

        if (head_len > 0) {
            const auto offset   = u32(sizeof(head)) - u32(head_len);
            mcpy(msg.data()+offset, head, head_len);
            StrView out{msg.data()+offset, msg.count()-offset};
            log_file.write(out);
        }
    }
}

}
