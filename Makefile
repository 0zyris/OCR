CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude -g -O0 -fsanitize=address `pkg-config --cflags sdl2 SDL2_image`
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

CUTTING_SRC = src/cutting_main.c src/image_processing.c src/cutting_letters.c src/tools.c
CUTTING_OBJ = src/cutting_main.o src/image_processing.o src/cutting_letters.o src/tools.o

SOLVER_SRC = src/solve/solver.c
LEARN_SRC = src/network/learn.c src/network/test.c lib/matrix.c

SOLVER_OBJ = src/solve/solver.o
LEARN_OBJ = src/network/learn.o src/network/test.o lib/matrix.o

OCR_SRC = src/main_ocr.c \
          src/gui/app.c \
          src/gui/layout.c \
          src/gui/callbacks.c \
          src/gui/image_view.c \
          src/image/image.c
OCR_OBJ = $(OCR_SRC:src/%.c=obj/%.o)

OCR_PKGS = gtk+-3.0 sdl2 SDL2_image
OCR_CFLAGS = $(shell pkg-config --cflags $(OCR_PKGS)) -Wall -Wextra -std=c11 -g
OCR_LDLIBS = $(shell pkg-config --libs $(OCR_PKGS)) -lm

all: solver learn cutting ocr

solver: $(SOLVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

learn: $(LEARN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

cutting: $(CUTTING_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

ocr: $(OCR_OBJ)
	$(CC) -o $@ $^ $(OCR_LDLIBS)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(OCR_CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SOLVER_OBJ) $(LEARN_OBJ) $(CUTTING_OBJ) solver learn cutting ocr image_storage/*.png
	rm -rf obj

.PHONY: all clean
