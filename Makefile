CXX = g++
#CFLAGS = -std=c++17

all: build run

build: 
	${CXX} ${CFLAGS} project.cxx 

debug: 
	${CXX} ${CFLAGS} project.cxx -DDEBUG

run:
	./a.out addresses.txt

clean:
	rm -f *.out
