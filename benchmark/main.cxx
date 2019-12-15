#include "rc/test.h"

#include "rc/env.h"

using namespace rc;

int main(int argc, const char* argv[]) {
  Vec<Str> args;
  for (int i = 1; i < argc; ++i) {
    args.push(Str::from_cstr(argv[i]));
  }

  test::run(args.as_slice());
  return 0;
}
