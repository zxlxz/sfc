#include "sfc/test.h"

using namespace sfc;

int main(int argc, const char* argv[]) {
  auto args = Vec<Str>{};
  for (auto idx = 0; idx < argc; ++idx) {
    args.push(Str::from(argv[idx]));
  }

  auto app = test::App{};
  app.run(args.as_slice());
  return 0;
}
