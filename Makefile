FLAGS=-WCL4 -pedantic -fsanitize=address -g -Iext/ -Isrc/ -I/Users/lucas/dev/rtree-tipe
SDL_FLAGS=$(shell sdl2-config --cflags --libs)

.PHONY: all clean

rect: src/rect.c src/rect.h src/constants.h
	gcc -c src/rect.c -o .build/rect.o $(FLAGS)

rtree: rect src/rtree.c src/rtree.h src/constants.h
	gcc -c src/rtree.c -o .build/rtree.o $(FLAGS)

main: rtree src/main.c src/main.h ext/gfx.h ext/dataset.h src/constants.h
	gcc src/main.c .build/rtree.o .build/rect.o -o .build/main.a $(FLAGS) $(SDL_FLAGS)

all: main
clean:
	rm -rf .build/rect.o .build/rtree.o .build/main.a