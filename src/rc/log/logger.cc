#include "rc.inl"

#include "rc/env.h"
#include "rc/log.h"

namespace rc::str {

using log::Level;

template<>
struct FromStr<Level> {
  static auto from_str(const Str& s) noexcept -> Option<Level> {
    static const Str names[] = {u8"Trace", u8"Debug", u8"Info",
                                u8"Warn",  u8"Error", u8"Fatal"};
    return str::_parse_enum<Level>(s, names);
  }
};

}

namespace rc::log {

pub auto Logger::buff() -> Vec<u8>& {
  static thread_local auto res = Vec<u8>::with_capacity(8192);
  res.clear();
  return res;
}

static auto _proc_start_time = time::Instant::now();
pub auto Logger::push(Level level, Slice<const u8> msg) -> void {
  const auto dur = _proc_start_time.elpased();
  const auto val = Entry{level, dur, msg};

  for (auto& backend : _backends) {
    backend(val);
  }
}

static auto _global_logger() -> Logger& {
  const auto level_str = env::var(u8"rc_log_level");
  const auto level_val = level_str.parse<Level>().unwrap_or(Level::Debug);

  static Logger res{level_val, {}};
  res._backends.reserve(2);

  /* stdout */
  res._backends.push(FnBox<void(Entry)>{[](Entry e) {
    static auto out = Stdout{};
    out.push(e);
  }});

  /* file */
  static auto log_path = env::var(u8"rc_log_path");
  if (!log_path.is_empty()) {
    res._backends.push(FnBox<void(Entry)>{[&](Entry e) {
      static auto out = File::create(log_path.as_str());
      out.push(e);
    }});
  }
  return res;
}

pub auto logger() -> Logger& {
  static auto& res = _global_logger();
  return res;
}

}  // namespace rc::log
