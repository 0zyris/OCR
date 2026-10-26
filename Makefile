CC = gcc
PKGS = gtk+-3.0 sdl2 SDL2_image
CPPFLAGS = $(shell pkg-config --cflags $(PKGS))
CFLAGS = -Wall -Wextra -std=c11 -g
LDLIBS = $(shell pkg-config --libs $(PKGS)) -lm

TARGET = ocr
SRC = src/main.c \
      src/gui/app.c \
      src/gui/layout.c \
      src/gui/callbacks.c \
      src/gui/image_view.c \
      src/image/image.c
OBJ = $(SRC:src/%.c=obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean
