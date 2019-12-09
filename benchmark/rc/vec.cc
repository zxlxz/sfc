#include "rc-benchmark.inl"

#include <string>
#include <typeinfo>
#include <vector>

namespace rc::vec {

struct EmplaceBackTest {
  test::Benchmark _manager;
  usize _cnt;

  EmplaceBackTest(Str name, usize cnt) : _manager(name, cnt), _cnt{cnt} {}

  template <class T, class... U>
  void std(const U&... args) {
    std::vector<T> stl_vec;
    stl_vec.reserve(_cnt);
    ::memset(stl_vec.data(), 0, stl_vec.size() * sizeof(T));
    _manager("stl ", [&]() { stl_vec.emplace_back(args...); });
  }

  template <class T>
  void rc(T val) {
    rc::vec::Vec<T> rc_vec;
    rc_vec.reserve(_cnt);
    ::memset(rc_vec.as_mut_ptr(), 0, rc_vec.len() * sizeof(T));
    _manager("rc", [&]() { rc_vec.push(val); });
  }

  void std_str(const char* p) {
    std::vector<String> stl_vec;
    stl_vec.reserve(_cnt);
    ::memset(stl_vec.data(), 0, stl_vec.size() * sizeof(String));
    _manager("stl ", [&]() { stl_vec.emplace_back(String::from(Str::from_cstr(p))); });
  }
};

rc_test(emplace_back) {
  usize cnt = 10 * 1024 * 1024;

  {
    EmplaceBackTest test_i32(u8"i32: ", cnt);
    test_i32.std<i32>(8);
    test_i32.rc<i32>(8);
    test_i32.std<i32>(8);
    test_i32.rc<i32>(8);
  }

  {
    EmplaceBackTest test_i64(u8"i64: ", cnt);
    test_i64.std<i64>(8);
    test_i64.rc<i64>(8);
    test_i64.std<i64>(8);
    test_i64.rc<i64>(8);
  }

  {
    EmplaceBackTest test_str(u8"str: short", cnt);
    test_str.std<std::string>("short");
    test_str.rc<std::string>("short");
    test_str.std<std::string>("short");
    test_str.rc<std::string>("short");
  }

  {
    EmplaceBackTest test_str(u8"std::string: long long long long long long long", cnt);
    test_str.std<std::string>("long long long long long long long");
    test_str.rc<std::string>("long long long long long long long");
    test_str.std<std::string>("long long long long long long long");
    test_str.rc<std::string>("long long long long long long long");
  }

}

}  // namespace rc::vec
