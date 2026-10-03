#include "learn.h"

Matrix *create_matrix(int rows, int cols){
//créer une matrice de x lignes et y colonnes
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = calloc(rows*cols,sizeof(double));
    return matrix;
}

void free_matrix(Matrix *m){
//free une matrice

    free(m->data);
    free(m);
    m = NULL;
}

void print_matrix(Matrix *X){
//Affiche une matrice

    int max_cols = X->cols;

    for (int i = 0; i< X->rows;i++){
        for (int j=0; j< X->cols;j++){
            printf("%.2f ",X->data[i*max_cols + j]);
        }
        printf("\n");
    }
}

Matrix *add_matrix(Matrix *m1, Matrix *m2){
//Addition de deux matrices -> Doivent avoir même dimensions

    if (m1->rows == m2->rows && m1->cols == m2->cols){
        Matrix *result = create_matrix(m1->rows,m2->cols);
        int max_cols = m1->cols;

        for (int i = 0; i< m1->rows;i++){
            for (int j=0; j< m1->cols;j++){
                result->data[i*max_cols + j] = m1->data[i*max_cols + j] + m2->data[i*max_cols +j];
            }
        }
        return result;
    }
    return NULL;
}

Matrix *mult_matrix(Matrix *m1, Matrix *m2){
//Multiplication de deux matrices

    if (m1->cols == m2->rows){
        Matrix *result = create_matrix(m1->rows,m2->cols);
        int max_cols = result->cols;

        for (int i = 0; i< result->rows;i++){
            for (int j=0; j< result->cols;j++){
                int sum = 0;
                for (int k = 0; k < m1->cols;k++){
                    sum = sum + m1->data[i*m1->cols + k] * m2->data[k*m1->cols + j];
                }
                result->data[i*max_cols + j] = sum;
            }
        }
        return result;
    }
    return NULL;
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
