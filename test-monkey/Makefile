test-monkey.so: read-write.c
	gcc -shared -fPIC -ldl $^ /usr/lib/liburing.a -o $@
clean:
	rm test-monkey.so
.PHONY: clean
