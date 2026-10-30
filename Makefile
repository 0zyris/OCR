CC = gcc

CFLAGS = -Wall -Wextra -Werror -Iinclude -g -O0 -fsanitize=address \
	`pkg-config --cflags sdl2 SDL2_image`

LDLIBS = `pkg-config --libs sdl2 SDL2_image`

APP_PKGS = gtk+-3.0 sdl2 SDL2_image
APP_CFLAGS = $(shell pkg-config --cflags $(APP_PKGS)) \
	-Wall -Wextra -Werror -std=c11 -g
APP_LDLIBS = $(shell pkg-config --libs $(APP_PKGS)) -lm


CUTTING_SRC = \
	src/letter_cutting/cutting_main.c \
	src/letter_cutting/image_processing.c \
	src/letter_cutting/cutting_letters.c \
	src/letter_cutting/cutting_tools.c

SOLVER_SRC = \
	src/solve/solver.c

LEARN_SRC = \
	src/network/learn.c \
	src/network/test.c \
	src/network/matrix.c

APP_SRC = \
	src/app/main_app.c \
	src/app/gui/app.c \
	src/app/gui/layout.c \
	src/app/gui/callbacks.c \
	src/app/gui/image_view.c \
	src/app/image/image.c


CUTTING_OBJ = $(CUTTING_SRC:src/%.c=obj/%.o)
SOLVER_OBJ  = $(SOLVER_SRC:src/%.c=obj/%.o)
LEARN_OBJ   = $(LEARN_SRC:src/%.c=obj/%.o)
APP_OBJ     = $(APP_SRC:src/%.c=obj/%.o)


all: solver learn cutting app

solver: $(SOLVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

learn: $(LEARN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

cutting: $(CUTTING_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

app: $(APP_OBJ)
	$(CC) -o $@ $^ $(APP_LDLIBS)


obj/app/%.o: src/app/%.c
	mkdir -p $(dir $@)
	$(CC) $(APP_CFLAGS) -c $< -o $@


obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f solver learn cutting app
	rm -rf obj
	rm -f image/image_storage/*.png
	rm -f image/image_storage/grid/*.png
	rm -f image/image_storage/word/*.png

.PHONY: all clean