#include <nms/core.h>
#include <nms/io/log.h>
#include <nms/io/console.h>
#include <nms/io/file.h>
#include <nms/util/system.h>
#include <nms/util/stacktrace.h>

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

StrView str_cast(Level level) {
    switch (level) {
    case Level::None:   return "none";
    case Level::Debug:  return "debug";
    case Level::Info:   return "info";
    case Level::Warn:   return "warning";
    case Level::Alert:  return "alert";
    case Level::Error:  return "error";
    case Level::Fatal:  return "fatal";
    }
    return "unknow";
}

class XmlFile
{
public:
    XmlFile()
    {}

    ~XmlFile() {
        if (txtfile_ == nullptr) {
            return;
        }
        close();
    }

    operator bool() const {
        return txtfile_ != nullptr;
    }

    void open(const Path& path, const Path& xslt) {
        if (txtfile_ != nullptr) {
            close();
        }
        if (path.str().isEmpty()) {
            return;
        }

        txtfile_ = new io::TxtFile{ path, File::Write };

        // write xml head
        txtfile_->write(cstr("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));

        // write xslt head
        if (!xslt.str().isEmpty()) {
            txtfile_->write("<?xml-stylesheet type=\"text/xsl\" href=\"");
            txtfile_->write(xslt);
            txtfile_->write("\"?>\n");
        }

        // begin log
        txtfile_->write(cstr("<log>\n"));
    }


    void close() {
        if (txtfile_ == nullptr) {
            return;
        }

        // end log
        txtfile_->write(cstr("</log>\n"));
        delete txtfile_;
    }

    void write(Level level, f64 time, StrView message) {
        if (txtfile_ == nullptr) {
            return;
        }

        txtfile_->write("  <item>\n");
        txtfile_->write("    <level>{}</level>\n",      str_cast(level));
        txtfile_->write("    <time>{}</time>\n",        time);
        txtfile_->write("    <message>{}</message>\n",  message);
        txtfile_->write("  </item>\n");
        txtfile_->sync();
    }

protected:
    String          attributes_;
    io::TxtFile*    txtfile_    = nullptr;
};

String  gXmlPath = {};
XmlFile gXmlFile = {};

NMS_API StrView getXmlPath() {
    return gXmlPath;
}

NMS_API void setXmlPath(const Path& path, const Path& xslt) {
    gXmlFile.open(path, xslt);
}

NMS_API void message(Level level, StrView msg) {

    if (level < gLevel) {
        return;
    }

    // current process time
    const auto time = clock();

    // 1. terminal
    {
        static const StrView titles[]   ={
            "[  ]",
            "\033[1;1m[--]",
            "\033[1;32m[**]",                     // green
            "\033[1;33m[??]",    "\033[1;43m[??]",// yellow
            "\033[1;31m[!!]",    "\033[1;41m[!!]" // red
        };
        const auto& title       = titles[u32(level)];
        char        head[64];
        auto        head_len    = snprintf(head, sizeof(head), "%*s%7.3f\033[0m ", title.count(), title.data(), time);


        StrView strs[] = {
            StrView(head, {u32(head_len)}),
            StrView(msg),
            StrView("\n")
        };
        console::writes(strs, numel(strs));

        if (level >= Level::Fatal) {
            CallStacks stacks;
            const auto n = stacks.count() - 4;
            for (u32 i = 0; i < n; ++i) {
                console::writeln("\t|-[{}] {}", i, stacks[i]);
            }
        }
    }

    // 2. xml
    if (gXmlFile) {
        gXmlFile.write(level, time, msg);
    }
}

}
