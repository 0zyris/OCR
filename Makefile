CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude -g -O0 -fsanitize=address `pkg-config --cflags sdl2 SDL2_image`
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

CUTTING_SRC = src/main.c src/image_processing.c src/cutting_letters.c src/tools.c
CUTTING_OBJ = src/main.o src/image_processing.o src/cutting_letters.o src/tools.o

SOLVER_SRC = src/solve/solver.c
LEARN_SRC = src/network/learn.c src/network/test.c lib/matrix.c

SOLVER_OBJ = src/solve/solver.o
LEARN_OBJ = src/network/learn.o src/network/test.o lib/matrix.o

all: solver learn cutting

solver: $(SOLVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

learn: $(LEARN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

cutting: $(CUTTING_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ ${LDLIBS}

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SOLVER_OBJ) $(LEARN_OBJ) $(CUTTING_OBJ) solver learn cutting image_storage/*.png

.PHONY: all clean

