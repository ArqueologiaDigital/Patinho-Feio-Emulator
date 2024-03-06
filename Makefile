OBJS = emulator
CFLAGS = -Wall -g -std=c++11
CC = g++
INCLUDE = -IC:\DEV\SDL2\x86_64-w64-mingw32\include
LIB = -LC:\DEV\SDL2\x86_64-w64-mingw32\lib
FLAGS = -lmingw32 -lSDL2main -lSDL2

SRCS = $(wildcard src/*.cpp)
ALL_FILES = $(wildcard src/*.cpp)

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -o $(OBJS) $(SRCS) $(FLAGS)

clean:
	rm -f $(OBJS)