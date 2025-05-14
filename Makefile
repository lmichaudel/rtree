.PHONY: all clean
FLAGS=-WCL4 -Wall -Wextra -pedantic -Iext/ -Isrc/ -Itool/
SDL_FLAGS=$(shell sdl2-config --cflags --libs)

#FLAGS+=-fsanitize=address -g
FLAGS+=-O3

rect: src/rect.c src/rect.h src/constants.h
	gcc -c src/rect.c -o .build/rect.o $(FLAGS)

rtree: rect src/rtree.c src/rtree.h src/constants.h
	gcc -c src/rtree.c -o .build/rtree.o $(FLAGS)

cdataset: tool/cdataset.cpp tool/json.hpp
	g++ tool/cdataset.cpp -o .build/cdataset.a $(FLAGS)

main: rtree src/main.c src/main.h ext/gfx.h src/constants.h
	gcc src/main.c .build/rtree.o .build/rect.o -o .build/main.a $(FLAGS) $(SDL_FLAGS) $(DEBUG_FLAGS)

all: main cdataset
clean:
	rm -rf .build/rect.o .build/rtree.o .build/main.a ./build/cdataset.a