CPPFLAGS = -O3
LDFLAGS = -lstdc++ -lglog -lgtest -lgtest_main

all: include lib/libcoding.a ldpc_test

clean:
	rm -f *.o ldpc_test
	rm -rf lib include

lib/libcoding.a: ldpc.o
	mkdir -p lib
	ar cr $@ $^

	g++ -o $@ $^ $(LDFLAGS)

.PHONY: include
include: ldpc.h
	mkdir -p include/coding
	cp $^ include/coding

ldpc_test: ldpc_test.o ldpc.o
	g++ -o $@ $^ $(LDFLAGS)

test: all
	valgrind -v --track-origins=yes --leak-check=full --show-reachable=yes ./ldpc_test --gtest_break_on_failure 

