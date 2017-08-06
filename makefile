
CXX				= clang++
CXXFLAGS		= -std=c++1z -frtti -fexceptions -g -fPIC -DNMS_BUILD -I. 

nms_lib_srcs	=  $(wildcard nms/*.cc nms/*/*.cc nms/*/*/*.cc)
nms_lib_objs	=  $(patsubst %.cc, %.o, $(nms_lib_srcs))

nms_test_srcs	= $(wildcard nms.test/*.cc)
nms_test_objs	= $(patsubst %.cc, %.o, $(nms_test_srcs))

all: bin/nms.test bin/libnms.dylib

bin/nms.test:		$(nms_test_objs)
	@mkdir -p bin/
	$(CXX) -o $@ $^

bin/libnms.dylib:	$(nms_lib_objs)
	@mkdir -p bin/
	$(CXX) -shared -o $@ $^

.o: %.cc

clean:
	@rm bin/nms.test bin/libnms.dylib
	@rm $(nms_lib_objs)
	@rm $(nms_test_objs)

test:
	cd test && ../bin/nms.test ../bin/libnms.dylib -nms::hpc::cuda;
.PHONY: all clean test
