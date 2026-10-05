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
    for (int i = 0; i < net->W1->rows * network->W1->cols; i++)
        network->W1->data[i] = random_weight();
    for (int i = 0; i < net->W2->rows * network->W2->cols; i++)
        network->W2->data[i] = random_weight();

    return network;
}


Matrix *linear_forward(Matrix *X, Matrix *W, Matrix *b){
    Matrix *z1 = mult_matrix(X,W);
    if (z1 == NULL)
    {
        return NULL;
    }
    Matrix *z2 = add_matrix(z1,b);
    free_matrix(z1);
    return z2;
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
