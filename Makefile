FLAGS=-WCL4 -pedantic -fsanitize=address -g -Iext/ -Isrc/ -I/Users/lucas/dev/rtree-tipe
SDL_FLAGS=$(shell sdl2-config --cflags --libs)

.PHONY: all clean

rtree: src/rtree.c src/rtree.h constants.h
	gcc -c src/rtree.c -o .build/rtree.o $(FLAGS)

main: src/main.c src/main.h ext/gfx.h ext/dataset.h rtree constants.h
	gcc src/main.c .build/rtree.o -o .build/main.a $(FLAGS) $(SDL_FLAGS)

all: main
clean:
	rm -rf rtree.o main.a