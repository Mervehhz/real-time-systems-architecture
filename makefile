# the compiler : gcc for c program, define as g++ program for c++

CC = g++

# compiler flags:
# -g adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings

CFLAGS = -g -Wall -Wextra -pedantic -std=c++17 -pthread
BIN = a.out
SOURCE = test.cpp
HEADER = Timer.h ITimer.h

# the build target executable :

TARGET : all

all: $(HEADER) $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE)

clean:
	rm a.out