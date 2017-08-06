#pragma once

#include <nms/core.h>

namespace nms::io::log
{

enum class Level
{
    None,
    Debug,
    Info,
    Warn,
    Alert,
    Error,
    Fatal,
};

extern Level gLevel;

/* show log message */
NMS_API void    message(Level level, StrView s);

/* set log level*/
NMS_API void    setLevel(Level level);

/* get log level */
NMS_API Level   getLevel();

/**
 * set xml log file
 * @param log_path:  the file path
 * @param xsl_path:  the xslt path
 *
 * if path is null, will close xml log file.
 * if xslt is null, will not use xslt
 */
NMS_API void    setXmlPath(const Path& log_path, const Path& xsl_path);

/**
 * get xml log file path
 */
NMS_API StrView getXmlPath();

template<class ...T>
__forceinline void message(Level level, StrView fmt, const T& ...args) {
    auto& buf = tlsString();
    buf.resize(0);
    sformat(buf, fmt, args...);
    message(level, buf);
}

/* nms.io.log: debug message */
template<class ...T> 
__forceinline void debug(StrView fmt, const T& ...args) {
    message(Level::Debug, fmt, args...);
}

/* nms.io.log: info message */
template<class ...T>
__forceinline void info (StrView fmt, const T& ...args) {
    message(Level::Info,  fmt, args...);
}

/* nms.io.log: warning message */
template<class ...T>
__forceinline void warn (StrView fmt, const T& ...args) {
    message(Level::Warn,  fmt, args...);
}

/* nms.io.log: alert message */
template<class ...T>
__forceinline void alert(StrView fmt, const T& ...args) {
    message(Level::Alert, fmt, args...);
}

/* nms.io.log: error message */
template<class ...T>
__forceinline void error(StrView fmt, const T& ...args) {
    message(Level::Error, fmt, args...); }

/* nms.io.log: fatal message */
template<class ...T>
__forceinline void fatal(StrView fmt, const T& ...args) {
    message(Level::Fatal, fmt, args...);
}

}
