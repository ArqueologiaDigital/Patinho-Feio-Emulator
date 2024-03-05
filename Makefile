OBJS = main
CFLAGS = -Wall -g -std=c++11
CC = g++
INCLUDE = -IC:\DEV\SDL2\i686-w64-mingw32\include
LIB = -LC:\DEV\SDL2\i686-w64-mingw32\lib
FLAGS = -lmingw32 -lSDL2main -lSDL2

all: $(OBJS)

$(OBJS): $(OBJS).cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -o $@ $< $(FLAGS)

clean:
	rm -f $(OBJS)