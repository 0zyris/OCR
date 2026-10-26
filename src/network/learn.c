#include "learn.h"

double random_weight(){
    //génère une valeur aléatoire entre -0.5 et 0.5
    return ((double)rand() / RAND_MAX) - 0.5;
}

Network *network_init(int input_size, int hidden_size, int output_size){
    Network *network = malloc(sizeof(Network));

    network->W1 = create_matrix(input_size, hidden_size);
    network->b1 = create_matrix(1, hidden_size);
    network->W2 = create_matrix(hidden_size, output_size);
    network->b2 = create_matrix(1, output_size);

    //On définit des poids aléatoires
    for (int i = 0; i < network->W1->rows * network->W1->cols; i++)
        network->W1->data[i] = random_weight();
    for (int i = 0; i < network->W2->rows * network->W2->cols; i++)
        network->W2->data[i] = random_weight();

    return network;
}

ForwardCache *forward(Network *network, Matrix *X){
    ForwardCache *cache = malloc(sizeof(ForwardCache));
    cache->z1 = linear_forward(X, network->W1, network->b1);
    cache->a1 = sigmoid(cache->z1);
    cache->z2 = linear_forward(cache->a1, network->W2, network->b2);
    cache->a2 = sigmoid(cache->z2);
    return cache;
}


Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b){
    Matrix *z= mult_matrix(X,W);
    if (z== NULL)
    {
        return NULL;
    }
    for (int i = 0; i < z->rows; i++){
        for (int j = 0; j < z->cols; j++){
            z->data[i*z->cols + j] += b->data[j];
        }
    }
    return z;
}

Matrix *sigmoid(Matrix *Z){
    Matrix *A = create_matrix(Z->rows,Z->cols);
    for (int i = 0; i < Z->rows; i++){
        for (int j=0; j< Z->cols; j++){
            A->data[i*Z->cols + j] = 1/(1 + exp(-Z->data[i*Z->cols + j]));
        }
    }
    return A;
}

double cost(Matrix *A, Matrix *y){
    double sum = 0;
    Matrix *log_A = fct_matrix(A,log);
    Matrix *YMultlog_A = mult_by_ele_matrix(log_A,y);
    Matrix *y1 = create_matrix(y->rows,y->cols);
    Matrix *A1= create_matrix(A->rows,A->cols);
    for (int i = 0; i < A->rows; i++){
        for (int j = 0; j < A->cols; j++){
            y1->data[i*y->cols +j] = 1-y->data[i*y->cols+j];
            A1->data[i*A->cols +j] = 1-A->data[i*A->cols+j];
        }
    }
    Matrix *log_A1 = fct_matrix(A1,log);
    Matrix *Y1Multlog_A1 = mult_by_ele_matrix(y1,log_A1);
    Matrix *costs = add_matrix(YMultlog_A,Y1Multlog_A1);
 
    for (int i = 0; i < costs->rows; i++){
        for (int j = 0; j < costs->cols; j++){
            sum = sum + costs->data[i*costs->cols +j];
        }
    }
    free_matrix(log_A);
    free_matrix(YMultlog_A);
    free_matrix(y1);
    free_matrix(A1);
    free_matrix(log_A1);
    free_matrix(Y1Multlog_A1);
    free_matrix(costs);
 
    return sum * (-1.0/A->rows);
}

Matrix *sigmoid_derivative(Matrix *A){
    Matrix *result = create_matrix(A->rows, A->cols);
    for (int i = 0; i < A->rows * A->cols; i++)
        result->data[i] = A->data[i] * (1.0 - A->data[i]);
    return result;
}

Gradients *backward(Network *network, ForwardCache *cache, Matrix *X, Matrix *y){
    Gradients *grads = malloc(sizeof(Gradients));
    double inv_m = 1.0 / X->rows;

    //couche de sortie
    Matrix *dz2 = sub_matrix(cache->a2, y);
    Matrix *a1_T = transpose_matrix(cache->a1);
    Matrix *tmp = mult_matrix(a1_T, dz2);
    grads->dW2 = scalar_mult_matrix(tmp, inv_m);
    free_matrix(tmp);
    tmp = sum_rows(dz2);
    grads->db2 = scalar_mult_matrix(tmp, inv_m);
    free_matrix(tmp);

    //remontée vers la couche cachée
    Matrix *W2_T = transpose_matrix(network->W2);
    Matrix *da1 = mult_matrix(dz2, W2_T);
    Matrix *deriv = sigmoid_derivative(cache->a1);
    Matrix *dz1 = mult_by_ele_matrix(da1, deriv);

    //couche cachée
    Matrix *X_T = transpose_matrix(X);
    tmp = mult_matrix(X_T, dz1);
    grads->dW1 = scalar_mult_matrix(tmp, inv_m);
    free_matrix(tmp);
    tmp = sum_rows(dz1);
    grads->db1 = scalar_mult_matrix(tmp, inv_m);
    free_matrix(tmp);

    free_matrix(dz2);
    free_matrix(a1_T);
    free_matrix(W2_T);
    free_matrix(da1);
    free_matrix(deriv);
    free_matrix(dz1);
    free_matrix(X_T);
    return grads;
}

void apply_update(Matrix **param, Matrix *grad, double lr){
    Matrix *scaled  = scalar_mult_matrix(grad, lr);
    Matrix *updated = sub_matrix(*param, scaled);
    free_matrix(scaled);
    free_matrix(*param);
    *param = updated;
}

void update_weights(Network *network, Gradients *grads, double lr){
    apply_update(&network->W1, grads->dW1, lr);
    apply_update(&network->b1, grads->db1, lr);
    apply_update(&network->W2, grads->dW2, lr);
    apply_update(&network->b2, grads->db2, lr);
}

void free_cache(ForwardCache *c){
    free_matrix(c->z1);
    free_matrix(c->a1);
    free_matrix(c->z2);
    free_matrix(c->a2);
    free(c);
}

void free_gradients(Gradients *g){
    free_matrix(g->dW1);
    free_matrix(g->db1);
    free_matrix(g->dW2);
    free_matrix(g->db2);
    free(g);
}

void free_network(Network *n){
    free_matrix(n->W1);
    free_matrix(n->b1);
    free_matrix(n->W2);
    free_matrix(n->b2);
    free(n);
}
