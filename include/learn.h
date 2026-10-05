#ifndef LEARN_H
#define LEARN_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "matrix.h"

Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b);
Matrix *sigmoid(Matrix *z);
//Matrix *sigmoid_derive(Matrix *a);
double cost(Matrix *A, Matrix *y);

#endif
