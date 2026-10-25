
CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g

PKGS = gtk+-3.0 sdl2 SDL2_image

CPPFLAGS = $(shell pkg-config --cflags $(PKGS))
LDLIBS = $(shell pkg-config --libs $(PKGS))

TARGET = ocr

SRC = src/main.c \
      src/gui/app.c \
      src/image/image.c

OBJ = $(SRC:src/%.c=obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj
	rm -f $(TARGET)

.PHONY: all clean
