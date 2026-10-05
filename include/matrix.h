#ifndef MATRIX_H
#define MATRIX_H

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
Matrix *sub_matrix(Matrix *m1, Matrix *m2);

Matrix *mult_matrix(Matrix *m1, Matrix *m2);
Matrix *scalar_mult_matrix(Matrix *m, double scal);
Matrix *mult_by_ele_matrix(Matrix *m1, Matrix *m2);

Matrix *fct_matrix(Matrix *m1, double (*fct)(double));
Matrix *transpose_matrix(Matrix *m);

#endif

