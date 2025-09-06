#include "sfc/app/opts.h"
#include "sfc/test/app.h"

using namespace sfc;

int main(int argc, const char* argv[]) {
  auto app = test::App{};
  app.main(argc, argv);
  return 0;
}
