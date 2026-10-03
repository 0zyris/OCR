#ifndef LEARN_H
#define LEARN_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

typedef struct {
    int rows;
    int cols;
    double *data;
} Matrix;

Matrix *create_matrix(int rows, int cols);
void free_matrix(Matrix *m);
void print_matrix(Matrix *X);
Matrix *add_matrix(Matrix *m1, Matrix *m2);
Matrix *mult_matrix(Matrix *m1, Matrix *m2);

Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b);

#endif
