CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude

SOLVER_SRC = src/solve/solver.c
LEARN_SRC = src/network/learn.c src/network/test.c lib/matrix.c

SOLVER_OBJ = src/solve/solver.o
LEARN_OBJ = src/network/learn.o src/network/test.o lib/matrix.o

all: solver learn

solver: $(SOLVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

learn: $(LEARN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SOLVER_OBJ) $(LEARN_OBJ) solver learn

.PHONY: all clean

