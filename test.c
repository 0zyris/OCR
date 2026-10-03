#include "learn.h"

int main(void){

    Matrix *X = create_matrix(2,2);
    Matrix *Y = create_matrix(2,2);
    X->data[0] = 1;
    X->data[2] = 1;
    Y->data[0] = 1;
    Y->data[2] = 1;
    
    printf("-------- X --------\n");
    print_matrix(X);
    printf("-------- Y --------\n");
    print_matrix(Y);

    Matrix *A = add_matrix(X,Y);
    Matrix *M = mult_matrix(X,Y);
    
    printf("-------- A --------\n");
    print_matrix(A);

    printf("-------- M --------\n");
    print_matrix(M);

    free_matrix(A);
    free_matrix(M);
    free_matrix(X);
    free_matrix(Y);
}
