./bin/Linux/main: src/main.cpp src/glad.c include/matrices.h include/utils.h include/tree.h include/tiny_obj_loader.h include/stb_image.h
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/Linux/main src/main.cpp src/glad.c src/tiny_obj_loader.cpp src/stb_image.cpp src/tree.cpp ./lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run
clean:
	rm -f bin/Linux/main

run: ./bin/Linux/main
	cd bin/Linux && ./main
