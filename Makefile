OBJS = ./build/emulator
CFLAGS = -Wall -g -std=c++11
CC = g++
INCLUDE = -IC:\DEV\SDL2\x86_64-w64-mingw32\include -I./include -I./imGUI
LIB = -LC:\DEV\SDL2\x86_64-w64-mingw32\lib
IMGUI_FLAGS = -ld3d9 -ldwmapi -lgdi32
FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lcomdlg32

SRCS = $(wildcard src/*.cpp)
SRCS += $(wildcard imGUI/*.cpp)
ALL_FILES = $(wildcard src/*.cpp)

all:
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -o $(OBJS) $(SRCS) $(FLAGS) $(IMGUI_FLAGS)

run:
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -o $(OBJS) $(SRCS) $(FLAGS) $(IMGUI_FLAGS)
	$(OBJS)

demo:
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -o demo src/demo.cxx $(wildcard imGUI/*.cpp) $(FLAGS) $(IMGUI_FLAGS)
	./demo

clean:
	rm -f $(OBJS)