CC = gcc
CFLAGS = -Wall -Wextra -g 'pkg-config --cflags sdl2 SDL2_image'
LDLIBS = 'pkg-config --libs sdl2 SDL2_image'

SRC = main.c
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