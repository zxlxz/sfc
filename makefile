
_CPPPATH        = -I.
_CPPDEFS        = -DNMS_BUILD
_CXXFLAGS       = -x c++ -std=c++1z -Wall -Wextra -Wno-unknown-pragmas -fPIC $(_CPPPATH) $(_CPPDEFS)

debug ?= 1
ifeq ($(debug), 1)
	CXXFLAGS    = $(_CXXFLAGS) -g
else
	CXXFLAGS    = $(_CXXFLAGS) -O2
endif

nms_lib_srcs    = $(wildcard nms/*.cc nms/*/*.cc nms/*/*/*.cc)
nms_lib_objs    = $(patsubst %.cc, %.o, $(nms_lib_srcs))

nms_test_srcs   = $(wildcard nms.test/*.cc)
nms_test_objs   = $(patsubst %.cc, %.o, $(nms_test_srcs))

nms_gch         = nms/core.h.gch

ifeq ($(shell uname), Darwin)
	nms_lib     = publish/bin/libnms.dylib
else
	nms_lib     = publish/bin/libnms.so
endif

nms_test        = publish/bin/nms.test

all: $(nms_lib) $(nms_test)

$(nms_lib): $(nms_gch) $(nms_lib_objs)
	@mkdir -p publish/bin/
	$(CXX) -shared -o $@ $(nms_lib_objs)

$(nms_test): $(nms_gch) $(nms_test_objs) $(nms_lib)
	@mkdir -p publish/bin/
	$(CXX) -o $@ $(nms_test_objs) -Lpublish/bin/ -lnms -lpthread -ldl

$(nms_gch): nms/core.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cc $(nms_gch)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test:
	cd publish/bin && LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):. ./nms.test +nms:: -nms::cuda::

clean:
	@rm -rf $(nms_lib) $(nms_test)
	@rm -rf $(nms_lib_objs)
	@rm -rf $(nms_test_objs)
	@rm -rf $(nms_gch)

.PHONY: all test clean
