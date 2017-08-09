
CXX				= g++

debug ?= 1
ifeq ($(debug), 1)
	CXXFLAGS		= -std=c++1z -frtti -fexceptions -g  -fPIC -DNMS_BUILD -I. 
else
	CXXFLAGS		= -std=c++1z -frtti -fexceptions -O2 -fPIC -DNMS_BUILD -I. 
endif

nms_lib_srcs	=  $(wildcard nms/*.cc nms/*/*.cc nms/*/*/*.cc)
nms_lib_objs	=  $(patsubst %.cc, %.o, $(nms_lib_srcs))

nms_test_srcs	= $(wildcard nms.test/*.cc)
nms_test_objs	= $(patsubst %.cc, %.o, $(nms_test_srcs))

ifeq ($(shell uname), Darwin)
	nms_lib 	= bin/libnms.dylib
else
	nms_lib		= bin/libnms.so
endif

nms_test		= bin/nms.test

all: $(nms_lib) $(nms_test)

$(nms_lib):	$(nms_lib_objs)
	@mkdir -p bin/
	$(CXX) -shared -o $@ $^

$(nms_test):		$(nms_test_objs)
	@mkdir -p bin/
	$(CXX) -o $@ $^

.o: %.cc

clean:
	@rm -rf $(nms_lib) $(nms_test)
	@rm -rf $(nms_lib_objs)
	@rm -rf $(nms_test_objs)

.PHONY: all clean
