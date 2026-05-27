#include "sfc/test/test.h"

namespace sfc::ptr::test {

struct Cnt {
  int ctor = 0;
  int dtor = 0;

 public:
  auto operator==(const Cnt& other) const {
    return ctor == other.ctor && dtor == other.dtor;
  }

  void fmt(auto& f) const {
    f.write_fmt(fmt::Args{"({},{})", ctor, dtor});
  }
};

struct RefCnt {
  Cnt* _cnt;

 public:
  RefCnt(Cnt& cnt) : _cnt{&cnt} {
    _cnt->ctor += 1;
  }

  ~RefCnt() {
    if (_cnt) _cnt->dtor += 1;
    _cnt = nullptr;
  }

  RefCnt(RefCnt&& other) noexcept : _cnt{other._cnt} {
    if (_cnt) _cnt->ctor += 1;
  }
};

SFC_TEST(unique) {
  using UPtr = ptr::Unique<int>;

  auto p1 = UPtr{};
  sfc::assert_eq(p1.ptr(), nullptr);

  auto val = 3;
  auto u2 = UPtr{&val};
  auto& u2_ptr = u2._ptr;
  sfc::assert_eq(u2.ptr(), &val);
  sfc::assert_eq(u2.ptr(), &val);

  auto u3 = mem::move(u2);
  sfc::assert_eq(u3.ptr(), &val);
  sfc::assert_eq(u2_ptr, nullptr);
}

SFC_TEST(write) {
  auto cnt = Cnt{};

  usize buf[1];
  auto ptr = ptr::cast_mut<RefCnt>(buf);
  {
    ptr::write(ptr, RefCnt{cnt});
    sfc::assert_eq(cnt, Cnt{2, 1});
  }
}

SFC_TEST(read) {
  auto cnt = Cnt{};

  auto t = RefCnt{cnt};
  sfc::assert_eq(cnt, Cnt{1, 0});
  {
    auto a = ptr::read(&t);
    sfc::assert_eq(cnt, Cnt{2, 1});
  }
  sfc::assert_eq(cnt, Cnt{2, 2});
}

SFC_TEST(write_bytes) {
  auto x = u32{0U};
  ptr::write_bytes(&x, 1, 1);
  sfc::assert_eq(x, 0x01010101U);
}

SFC_TEST(drop) {
  auto cnt = Cnt{};

  {
    auto t = RefCnt{cnt};
    sfc::assert_eq(cnt, Cnt{1, 0});
    ptr::drop(&t);
    sfc::assert_eq(cnt, Cnt{1, 1});
  }
}

SFC_TEST(copy) {
  int v[] = {1, 1, 2, 2};
  ptr::copy(v, v + 2, 2);
  sfc::assert_eq(v[0], 1);
  sfc::assert_eq(v[1], 1);
  sfc::assert_eq(v[2], 1);
  sfc::assert_eq(v[3], 1);
}

SFC_TEST(copy_nonoverlapping) {
  int v[] = {1, 1, 2, 2};
  ptr::copy_nonoverlapping(v, v + 2, 2);
  sfc::assert_eq(v[0], 1);
  sfc::assert_eq(v[1], 1);
  sfc::assert_eq(v[2], 1);
  sfc::assert_eq(v[3], 1);
}

SFC_TEST(shift_elements_left) {
  int v[] = {0, 1, 2, 3, 4};
  ptr::shift_elements_left(v + 1, 4, 1);
  sfc::assert_eq(v[0], 1);
  sfc::assert_eq(v[1], 2);
  sfc::assert_eq(v[2], 3);
  sfc::assert_eq(v[3], 4);
  sfc::assert_eq(v[4], 4);
}

SFC_TEST(shift_elements_right) {
  int v[] = {0, 1, 2, 3, 4};
  ptr::shift_elements_right(v, 4, 1);
  sfc::assert_eq(v[0], 0);
  sfc::assert_eq(v[1], 0);
  sfc::assert_eq(v[2], 1);
  sfc::assert_eq(v[3], 2);
  sfc::assert_eq(v[4], 3);
}

}  // namespace sfc::ptr::test
