#pragma once

#include "rc/core.h"

namespace rc::term {

struct Color {
  enum Code : u8 {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    DEFAULT = 9,
  };
  Code _code;

  // sign: -+
  // fill: *=
  template <class Out>
  auto fmt(fmt::Formatter<Out>& formatter) const -> void {
    formatter.write_str(u8"\033[");
    formatter.write_chr(formatter.fill() == u8'=' ? u8'4' : u8'3');
    formatter.write_chr(u8('0' + _code));
  }
};

struct Font {
  enum Code : i8 {
    BOLD = 1,
    ITALICS = 3,
    UNDERLINE = 4,
    SLOW_BLINK = 5,
    FAST_BLINK = 6,
    REVERSE = 7,
    ERASE = 8,
    STRIKETHOUGH = 9,
  };
  Code _code;

  template <class Out>
  auto fmt(fmt::Formatter<Out>& formatter) const -> void {
    // +-
    const auto cond = formatter.sign() != u8'-';

    if (formatter.sign() == u8'-') {
      formatter.write_chr(u8'2');
    }
    formatter.write_chr(u8(u8'0' + _code));
  }
};

struct Style {
  Option<Font> _font;
  Option<Color> _fg_color;
  Option<Color> _bg_color;

  auto operator|(const Style& b) const -> Style {
    return Style{b._font || _font, b._fg_color || _fg_color,
                 b._bg_color || _bg_color};
  }

  template <class Out>
  auto fmt(fmt::Formatter<Out>& formatter) const -> void {
    if (_font.is_some()) {
      return;
    }
    if (_fg_color.is_some()) {
      formatter.write(u8"\x1b[3{}m", u32(_fg_color._val._code));
    }
    if (_bg_color.is_some()) {
      formatter.write(u8"\x1b[4{}m", u32(_fg_color._val._code));
    }
  }
};

inline auto font(Font::Code code) -> Style {
  return Style{._font = Font{code}};
}

inline auto fg(Color::Code code) -> Style {
  return Style{._fg_color = Color{code}};
}

inline auto bg(Color::Code code) -> Style {
  return Style{._bg_color = Color{code}};
}

static auto FG_BLACK = Style{._fg_color = Color{Color::BLACK}};
static auto FG_RED = Style{._fg_color = Color{Color::RED}};
static auto FG_GREEN = Style{._fg_color = Color{Color::GREEN}};
static auto FG_YELLOW = Style{._fg_color = Color{Color::YELLOW}};
static auto FG_BLUE = Style{._fg_color = Color{Color::BLUE}};
static auto FG_CYAN = Style{._fg_color = Color{Color::CYAN}};
static auto FG_WHITE = Style{._fg_color = Color{Color::WHITE}};
static auto FG_DEFAULT = Style{._fg_color = Color{Color::DEFAULT}};

static auto BG_BLACK = Style{._bg_color = Color{Color::BLACK}};
static auto BG_RED = Style{._bg_color = Color{Color::RED}};
static auto BG_GREEN = Style{._bg_color = Color{Color::GREEN}};
static auto BG_YELLOW = Style{._bg_color = Color{Color::YELLOW}};
static auto BG_BLUE = Style{._bg_color = Color{Color::BLUE}};
static auto BG_CYAN = Style{._bg_color = Color{Color::CYAN}};
static auto BG_WHITE = Style{._bg_color = Color{Color::WHITE}};
static auto BG_DEFAULT = Style{._bg_color = Color{Color::DEFAULT}};

}  // namespace rc::term
