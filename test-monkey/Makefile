test-monkey.so: read-write.cpp
	# or pkg-config
	g++ -std=c++20 -shared -fPIC -ldl $^ /usr/lib/liburing.a -o $@
clean:
	rm test-monkey.so
.PHONY: clean
