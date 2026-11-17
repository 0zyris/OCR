#ifndef LEARN_H
#define LEARN_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "matrix.h"

typedef struct {
    Matrix *W1; //weights hidden layer
    Matrix *b1; //bias hidden layer
    Matrix *W2; //weights out layer
    Matrix *b2; //weights out layer
} Network;

typedef struct {
    Matrix *z1;
    Matrix *a1;
    Matrix *z2;
    Matrix *a2;
} ForwardCache;

typedef struct {
    Matrix *dW1;
    Matrix *db1;
    Matrix *dW2;
    Matrix *db2;
} Gradients;

Network *network_init(int input_size, int hidden_size, int output_size);
Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b);
Matrix *sigmoid(Matrix *z);

ForwardCache *forward(Network *network, Matrix *X);
Matrix *sigmoid_derive(Matrix *a);
double cost(Matrix *A, Matrix *y);
Gradients *backward(Network *network, ForwardCache *cache, Matrix *X, Matrix *y);
void apply_update(Matrix **param, Matrix *grad, double lr);
void update_weights(Network *network, Gradients *grads, double lr);

void free_cache(ForwardCache *c);
void free_gradients(Gradients *g);
void free_network(Network *n);
#endif
