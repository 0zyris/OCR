CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -fsanitize=address `pkg-config --cflags sdl2 SDL2_image`
LDLIBS = `pkg-config --libs sdl2 SDL2_image`

SRC = src/main.c src/image_processing.c src/cutting_letters.c src/tools.c
OBJ = ${SRC:.c=.o}

EXEC = solver


all: ${EXEC}

${EXEC}: ${OBJ}
	${CC} ${CFLAGS} -o $@ $^ ${LDLIBS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o$@

clean:
	rm -f ${OBJ} ${EXEC}


.PHONY: all clean