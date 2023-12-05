test-monkey.so: read-write.cpp
	# TODO: use pkg-config
	# remember to build liburing with -ggdb and -O0 as well
	# (it uses -g -O3 by default)
	g++ -ggdb -O0 -std=c++20 -shared -fPIC -ldl $^ /usr/lib/liburing.a -o $@
test: multi-threaded-test.cpp
	g++ -ggdb -O0 -std=c++20 $^ -o $@
clean:
	rm test-monkey.so
.PHONY: clean
