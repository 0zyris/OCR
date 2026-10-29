#ifndef SOLVER_H
#define SOLVER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct {
    char *cells;
    int rows;
    int cols;
} Grid;

void free_grid(Grid *grid);
Grid *init_grid(size_t rows, size_t cols);
void print_grid(Grid *G);
Grid *get_grid(char* filename);
int find_word(const Grid *g, const char *word, 
        int *x1, int *y1, int *x2, int *y2);


#endif
