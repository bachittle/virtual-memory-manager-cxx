CXX = g++
#CFLAGS = -std=c++17

all: build run test

build: 
	${CXX} ${CFLAGS} project.cxx 

debug: 
	${CXX} ${CFLAGS} project.cxx -DDEBUG

run:
	./a.out addresses.txt

test:
	@echo "-- If no lines follow diff, this means that the files are identical, otherwise there is an anomaly. --"
	diff -u result.txt correct.txt

clean:
	rm -f *.out
