#include "sfc/test.h"

using namespace sfc;

int main(int argc, cstr_t argv[]) {
  auto args = Vec<Str>{};
  for (auto idx = 0; idx < argc; ++idx) {
    args.push(Str{argv[idx]});
  }

  auto app = test::App{};
  app.run(args.as_slice());
  return 0;
}
