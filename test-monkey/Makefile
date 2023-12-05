CXX = g++
CXXFLAGS = -ggdb -O0
CXXFLAGS += -std=c++20
CXXFLAGS += -shared -fPIC
LDFLAGS = -ldl
libmonkey.so: monkey.cpp
	# TODO: use pkg-config
	# remember to build liburing with -ggdb and -O0 as well
	# (it uses -g -O3 by default)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ /usr/lib/liburing.a -o $@
test: multi-threaded-test.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
clean:
	rm test-monkey.so
.PHONY: clean
