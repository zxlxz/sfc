#include "sfc/log.h"

#include "sfc/log/backend/console.h"
#include "sfc/time.h"

namespace sfc::log {

class LogTime {
  char _buf[24] = "0000-00-00T00:00:00.000";

  time::System _sys_time{};
  time::DateTime _day_time{};

 public:
  auto to_str() const -> Str {
    return Str{_buf, sizeof(_buf) - 1};
  }

  void update() {
    const auto cur_time = time::System::now();

    if (cur_time.secs() != _sys_time.secs()) {
      _day_time = time::DateTime::from_local(cur_time);
      this->fill_date();
      this->fill_time();
    }

    const auto sub_millis = cur_time.sub_nanos() / time::NANOS_PER_MILLI;
    this->fill_millis(static_cast<u32>(sub_millis));
  }

 private:
  template <u32 N>
  void pad_int(u32 val, char* p) {
    for (auto i = 0U; i < N; ++i) {
      p[N - 1 - i] = static_cast<char>('0' + val % 10);
      val /= 10;
    }
  }

  void fill_date() {
    this->pad_int<4>(_day_time.year, _buf);
    this->pad_int<2>(_day_time.month, _buf + sizeof("0000"));
    this->pad_int<2>(_day_time.day, _buf + sizeof("0000-00"));
  }

  void fill_time() {
    this->pad_int<2>(_day_time.hour, _buf + sizeof("0000-00-00T00"));
    this->pad_int<2>(_day_time.minute, _buf + sizeof("0000-00-00T00:00"));
    this->pad_int<2>(_day_time.second, _buf + sizeof("0000-00-00T00:00"));
  }

  void fill_millis(u32 sub_millis) {
    this->pad_int<3>(sub_millis, _buf + sizeof("0000-00-00 00:00:00"));
  }
};

Logger::Logger() = default;

Logger::~Logger() noexcept= default;

auto Logger::get_level() const -> Level {
  return _level;
}

void Logger::set_level(Level level) {
  _level = level;
}

void Logger::flush() {
  for (auto& be : _backends.as_mut_slice()) {
    be->flush();
  }
}

void Logger::write_msg(Level level, Str msg) {
  static thread_local auto time = LogTime{};

  if (level < _level) {
    return;
  }

  time.update();
  const auto entry = Record{
      .level = level,
      .time = time.to_str(),
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write(entry);
  }
}

auto Logger::tls_buf() -> String& {
  static thread_local auto tls_buf = String::with_capacity(1024);
  tls_buf.clear();
  return tls_buf;
}

Logger& global() {
  static auto res = Logger{};

  [[maybe_unused]] static auto static_init = [] {
    res.set_level(Level::Info);
    res.add_backend(ConsoleBackend{});
    return true;
  }();

  return res;
}

}  // namespace sfc::log
