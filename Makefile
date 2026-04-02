all: msr

msr: MSR605.cpp libmsr605.a
	g++ -Wall -static MSR605.cpp libmsr605.a -o msr605

msr605.o: libmsr605.cpp
	g++ -Wall -c libmsr605.cpp -o msr605.o

libmsr605.a: msr605.o
	ar rcs libmsr605.a msr605.o

clean:
	rm -rf libmsr605.so test msr605.o msr605
