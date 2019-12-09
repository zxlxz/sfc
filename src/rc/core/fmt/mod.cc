#include "rc.inl"

#include "rc/core/fmt.h"
#include "rc/core/macros.h"

namespace rc::fmt {

enum class CharKind {
  Null,
  Begin,
  End,
  Fill,
  Align,
  Sign,
  Prefix,
  Point,
  Number,
  Type,
};


struct CharId {
  u8 _val;

  auto kind() const noexcept -> CharKind {
    switch (_val) {
      case u8'{':
        return CharKind::Begin;
      case u8'}':
        return CharKind::End;
      case u8'<':
      case u8'>':
      case u8'^':
      case u8'=':
        return CharKind::Align;
      case u8'+':
      case u8'-':
        return CharKind::Sign;
      case u8'#':
        return CharKind::Prefix;
      case u8'.':
        return CharKind::Point;
      case u8'0':
      case u8'1':
      case u8'2':
      case u8'3':
      case u8'4':
      case u8'5':
      case u8'6':
      case u8'7':
      case u8'8':
      case u8'9':
        return CharKind::Number;
      default:
        return CharKind::Type;
    }
  }
};

template <class T>
struct IntoStyle;

template <>
struct IntoStyle<Str> {
  const u8* _itr;
  const u8* _end;

  explicit IntoStyle(Str s) noexcept
      : _itr{s.as_ptr()}, _end{s.as_ptr() + s.len()} {}

  auto len() const noexcept -> usize { return usize(_end - _itr); }

  auto is_empty() const noexcept { return _itr >= _end; }

  auto next_char() noexcept -> u8 {
    if (this->is_empty()) {
      return 0;
    }
    return *_itr++;
  }

  auto kind() const noexcept -> CharKind {
    if (_itr >= _end) {
      return CharKind::Null;
    }

    const auto k0 = CharId{_itr[0]}.kind();
    if (k0 == CharKind::Type) {
      auto k1 = CharId{_itr[1]}.kind();
      if (k1 == CharKind::Align) {
        return CharKind::Fill;
      }
    }
    return k0;
  }

  auto into_style() -> Style {
    if (this->is_empty()) {
      return Style{};
    }
    auto res = Style{};

    // [[fill]align]
    if (this->kind() == CharKind::Fill) {
      res._fill = this->next_char();
      res._align = this->next_char();
    } else if (this->kind() == CharKind::Align) {
      res._align = this->next_char();
    }

    // [sign]
    if (this->kind() == CharKind::Sign) {
      res._sign = this->next_char();
    }

    // [prefix]
    if (this->kind() == CharKind::Prefix) {
      res._prefix = this->next_char();
    }

    // [width]
    int width = 0;
    while (this->kind() == CharKind::Number) {
      const auto n = this->next_char() - u8'0';
      width = width * 10 + n;
    }
    res._width = u8(width);

    // [.prec]
    if (this->kind() == CharKind::Point) {
      res._point = this->next_char();

      int prec = 0;
      while (this->kind() == CharKind::Number) {
        const auto n = this->next_char() - u8'0';
        prec = prec * 10 + n;
      }
      res._prec = u8(prec);
    }

    // [type][spec]
    if (this->kind() != CharKind::End) {
      res._type = this->next_char();
    }

    return res;
  }
};

pub auto Style::from(Str s) -> Style {
  auto x = IntoStyle<Str>{s};
  return x.into_style();
}

}  // namespace rc::fmt
