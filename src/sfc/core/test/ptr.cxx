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
    f.write_fmt("({},{})", ctor, dtor);
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
  sfc::expect_eq(p1.ptr(), nullptr);

  auto val = 3;
  auto u2 = UPtr{&val};
  auto& u2_ptr = u2._ptr;
  sfc::expect_eq(u2.ptr(), &val);

  sfc::expect_eq(u2.clone().ptr(), &val);
  sfc::expect_eq(u2.ptr(), &val);

  auto u3 = static_cast<UPtr&&>(u2);
  sfc::expect_eq(u3.ptr(), &val);
  sfc::expect_eq(u2_ptr, nullptr);
}

SFC_TEST(write) {
  auto cnt = Cnt{};

  usize buf[1];
  auto ptr = reinterpret_cast<RefCnt*>(buf);
  {
    ptr::write(ptr, RefCnt{cnt});
    sfc::expect_eq(cnt, Cnt{2, 1});
  }
  sfc::expect_eq(cnt, Cnt{2, 1});
  ptr->~RefCnt();
  sfc::expect_eq(cnt, Cnt{2, 2});
}

SFC_TEST(read) {
  auto cnt = Cnt{};

  {
    auto t = RefCnt{cnt};
    sfc::expect_eq(cnt, Cnt{1, 0});
    {
      auto a = ptr::read(&t);
      sfc::expect_eq(cnt, Cnt{2, 0});
    }
    sfc::expect_eq(cnt, Cnt{2, 1});
  }
  sfc::expect_eq(cnt, Cnt{2, 2});
}

SFC_TEST(write_bytes) {
  auto x = u32{0U};
  ptr::write_bytes(&x, 1, 1);
  sfc::expect_eq(x, 0x01010101U);
}

SFC_TEST(drop) {
  auto cnt = Cnt{};

  {
    auto t = RefCnt{cnt};
    sfc::expect_eq(cnt, Cnt{1, 0});
    ptr::drop(&t);
    sfc::expect_eq(cnt, Cnt{1, 1});
  }
}

SFC_TEST(copy) {
  u32 v[] = {1, 1, 2, 2};
  ptr::copy(v, v + 2, 2);
  sfc::expect_eq(v[0], 1);
  sfc::expect_eq(v[1], 1);
  sfc::expect_eq(v[2], 1);
  sfc::expect_eq(v[3], 1);
}

SFC_TEST(copy_nonoverlapping) {
  u32 v[] = {1, 1, 2, 2};
  ptr::copy_nonoverlapping(v, v + 2, 2);
  sfc::expect_eq(v[0], 1);
  sfc::expect_eq(v[1], 1);
  sfc::expect_eq(v[2], 1);
  sfc::expect_eq(v[3], 1);
}

SFC_TEST(shift_elements_left) {
  u32 v[] = {0, 1, 2, 3, 4};
  ptr::shift_elements_left(v + 1, 4, 1);
  sfc::expect_eq(v[0], 1);
  sfc::expect_eq(v[1], 2);
  sfc::expect_eq(v[2], 3);
  sfc::expect_eq(v[3], 4);
  sfc::expect_eq(v[4], 4);
}

SFC_TEST(shift_elements_right) {
  u32 v[] = {0, 1, 2, 3, 4};
  ptr::shift_elements_right(v, 4, 1);
  sfc::expect_eq(v[0], 0);
  sfc::expect_eq(v[1], 0);
  sfc::expect_eq(v[2], 1);
  sfc::expect_eq(v[3], 2);
  sfc::expect_eq(v[4], 3);
}

}  // namespace sfc::ptr::test
