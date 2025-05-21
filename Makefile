.PHONY: all clean
FLAGS=-WCL4 -Wall -Wextra -pedantic -Isrc/
SDL_FLAGS=$(shell sdl2-config --cflags --libs)

#FLAGS+=-fsanitize=address -g
FLAGS+=-O3

rect: src/rect.c src/rect.h src/constants.h
	gcc -c src/rect.c -o .build/rect.o $(FLAGS)

rtree: rect src/item.h src/rtree.c src/rtree.h src/constants.h
	gcc -c src/rtree.c -o .build/rtree.o $(FLAGS)

quadtree: rect src/item.h src/competitors/quadtree.c src/competitors/quadtree.h
	gcc -c src/competitors/quadtree.c -o .build/quadtree.o $(FLAGS)

cdataset: tool/cdataset.cpp tool/json.hpp
	g++ tool/cdataset.cpp -o .build/cdataset.a $(FLAGS) -Itool/

demo: rtree quadtree demo/main.c demo/main.h demo/gfx.h demo/stb_image_write.h src/constants.h
	gcc demo/main.c .build/quadtree.o .build/rtree.o .build/rect.o -o .build/main.a -Idemo/ $(FLAGS) $(SDL_FLAGS)

benchmark: rtree quadtree benchmark/main.c benchmark/main.h
	gcc benchmark/main.c .build/rtree.o .build/quadtree.o .build/rect.o -o .build/benchmark.a -Ibenchmark/ $(FLAGS)

all: demo cdataset benchmark
clean:
	rm -rf .build/