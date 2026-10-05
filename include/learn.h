#ifndef LEARN_H
#define LEARN_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "matrix.h"

typedef struct {
    Matrix *W1; //poids de couche cachée
    Matrix *b1; //biais de couche cachée
    Matrix *W2; //poids de couche de sortie
    Matrix *b2; //biais de couche de sortie
} Network;

Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b);
Matrix *sigmoid(Matrix *z);
//Matrix *sigmoid_derive(Matrix *a);
double cost(Matrix *A, Matrix *y);

#endif
