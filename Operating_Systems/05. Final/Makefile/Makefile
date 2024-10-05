CC = gcc
CFLAGS =

runner: runner.c libdummy.so
    $(CC) $(CFLAGS) -o runner.out $< -L. -ldummy -Wl,-rpath,.

test: test_runner.c libdummy.so
    $(CC) $(CFLAGS) -o test_runner.out $< -L. -ldummy -Wl,-rpath,.
    ./test_runner.out

libdummy.so: libdummy.c
    $(CC) $(CFLAGS) -shared -fPIC -o $@ $<

clean:
    rm -f runner.out test_runner.out libdummy.so