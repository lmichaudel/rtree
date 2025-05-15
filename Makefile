.PHONY: all clean
FLAGS=-WCL4 -Wall -Wextra -pedantic -Isrc/ -Idemo/
SDL_FLAGS=$(shell sdl2-config --cflags --libs)

#FLAGS+=-fsanitize=address -g
FLAGS+=-O3

rect: src/rect.c src/rect.h src/constants.h
	gcc -c src/rect.c -o .build/rect.o $(FLAGS)

rtree: rect src/rtree.c src/rtree.h src/constants.h
	gcc -c src/rtree.c -o .build/rtree.o $(FLAGS)

cdataset: tool/cdataset.cpp tool/json.hpp
	g++ tool/cdataset.cpp -o .build/cdataset.a $(FLAGS) -Itool/

main: rtree demo/main.c demo/main.h demo/gfx.h demo/stb_image_write.h src/constants.h
	gcc demo/main.c .build/rtree.o .build/rect.o -o .build/main.a $(FLAGS) $(SDL_FLAGS) $(DEBUG_FLAGS)

benchmark: rtree benchmark/main.c benchmark/main.h
	gcc benchmark/main.c $(FLAGS)

all: main cdataset
clean:
	rm -rf .build/rect.o .build/rtree.o .build/main.a ./build/cdataset.a