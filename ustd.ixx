
/* c++ -> rust */
#define let const auto
#define mut auto
#define fn  auto
#define use using

module ustd;

export {
#include <ustd/core.h>
#include <ustd/boxed.h>
#include <ustd/fs.h>
#include <ustd/io.h>
#include <ustd/path.h>
#include <ustd/sync.h>
#include <ustd/test.h>
#include <ustd/thread.h>
#include <ustd/time.h>
}
