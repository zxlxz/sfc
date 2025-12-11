#include "sfc/log.h"

#include "sfc/log/console_backend.h"
#include "sfc/time.h"

namespace sfc::log {

class LogTime {
  char _buf[24] = "0000-00-00T00:00:00.000";

  time::SysTime _sys_time{};
  time::DateTime _day_time{};

 public:
  auto to_str() const -> Str {
    return Str{_buf, sizeof(_buf) - 1};
  }

  void update() {
    const auto cur_time = time::SysTime::now();

    if (cur_time.secs() != _sys_time.secs()) {
      _day_time = time::DateTime::from_local(cur_time);
      this->fill_date();
      this->fill_time();
    }

    this->fill_millis(cur_time.subsec_millis());
  }

 private:
  void pad_int(u32 val, Slice<char> buf) {
    for (auto i = 0U; i < buf._len; ++i) {
      buf._ptr[buf._len - 1 - i] = static_cast<char>('0' + val % 10);
      val /= 10;
    }
  }

  void fill_date() {
    this->pad_int(_day_time.year, {_buf, 4});
    this->pad_int(_day_time.month, {_buf + sizeof("YYYY"), 2});
    this->pad_int(_day_time.day, {_buf + sizeof("YYYY-MM"), 2});
  }

  void fill_time() {
    this->pad_int(_day_time.hour, {_buf + sizeof("YYYY-MM-DD"), 2});
    this->pad_int(_day_time.minute, {_buf + sizeof("YYYY-MM-DDTHH"), 2});
    this->pad_int(_day_time.second, {_buf + sizeof("YYYY-MM-DDTHH:MM"), 2});
  }

  void fill_millis(u32 sub_millis) {
    this->pad_int(sub_millis, {_buf + sizeof("YYYY-MM-DDTHH:MM:SS"), 3});
  }
};

auto Record::time_str() -> Str {
  static thread_local auto time = LogTime{};
  time.update();
  return time.to_str();
}

auto Record::tls_buf() -> String& {
  static thread_local auto res = String::with_capacity(1024);
  res.clear();
  return res;
}

Logger& global() {
  auto make_logger = [] -> Logger {
    auto logger = Logger{};
    logger.set_level(Level::Info);
    logger.add_backend(ConsoleBackend{});
    return logger;
  };
  
  static auto res = make_logger();
  return res;
}

}  // namespace sfc::log
