#include <nms/test.h>

namespace nms
{

namespace core
{

nms_test(nmscpp_str) {
    io::log::debug("NMSCPP_STR(123     ) => `{}`", NMSCPP_STR(123));
    io::log::debug("NMSCPP_STR(__LINE__) => `{}`", NMSCPP_STR(__LINE__));
}

nms_test(nmscpp_at) {
    io::log::debug("NMSCPP_AT(0, a, b, c) => `{}`", NMSCPP_AT(0, "a", "b", "c"));
    io::log::debug("NMSCPP_AT(1, a, b, c) => `{}`", NMSCPP_AT(1, "a", "b", "c"));
    io::log::debug("NMSCPP_AT(2, a, b, c) => `{}`", NMSCPP_AT(2, "a", "b", "c"));
}

nms_test(nmscpp_count) {
    io::log::debug("NMSCPP_COUNT(       ) => `{}`", NMSCPP_COUNT(       ));
    io::log::debug("NMSCPP_COUNT(a      ) => `{}`", NMSCPP_COUNT(a      ));
    io::log::debug("NMSCPP_COUNT(a, b   ) => `{}`", NMSCPP_COUNT(a, b   ));
    io::log::debug("NMSCPP_COUNT(a, b, c) => `{}`", NMSCPP_COUNT(a, b, c));
}

nms_test(nmscpp_for) {
    io::log::debug("NMSCPP_FOR(MSG, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9)");

#define NMS_CORE_FOR_MSG(x)     io::log::debug("  => {}", x);
    NMSCPP_FOR(NMS_CORE_FOR_MSG, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
#undef  NMS_CORE_FOR_MSG
}

nms_test(nmscpp_loop) {
    io::log::debug(R"(NMSCPP_LOOP(10, MSG, "hello world"))");

#define NMS_CORE_LOOP_MSG(id, msg)     io::log::debug("  {} => {}", id, msg);
    NMSCPP_LOOP(10, NMS_CORE_LOOP_MSG, "hello world");
#undef  NMS_CORE_LOOP_MSG
}


}

}