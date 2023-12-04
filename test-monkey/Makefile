test-monkey.so: read-write.cpp
	# or pkg-config
	g++ -std=c++20 -shared -fPIC -ldl $^ /usr/lib/liburing.a -o $@
test: multi-threaded-test.cpp
	g++ -ggdb -O0 -std=c++20 $^ -o $@
clean:
	rm test-monkey.so
.PHONY: clean
