#include "matrix.h"


Matrix *create_matrix(int rows, int cols){
//créer une matrice de x lignes et y colonnes
    Matrix *matrix = malloc(sizeof(Matrix));

    if (matrix == NULL)
    {
        return NULL;
    }

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

//================== Opérations ========================//

Matrix *add_matrix(Matrix *m1, Matrix *m2){
//Addition de deux matrices -> Doivent avoir même dimensions

    if (m1->rows != m2->rows || m1->cols != m2->cols){
        return NULL;
    }

    Matrix *result = create_matrix(m1->rows,m2->cols);
    int max_cols = m1->cols;

    for (int i = 0; i< m1->rows;i++){
        for (int j=0; j< m1->cols;j++){
            double d1 = m1->data[i*max_cols +j];
            double d2 = m2->data[i*max_cols +j];
            result->data[i*max_cols + j] = d1 + d2;
        }
    }
    return result;
}

Matrix *sub_matrix(Matrix *m1, Matrix *m2){
//Soustration de deux matrices -> Doivent avoir même diemnsions
    //
    if (m1->rows != m2->rows || m1->cols != m2->cols){
        return NULL;
    }

    int max_cols = m1->cols;

    Matrix *result = create_matrix(m1->rows, m1->cols);

    for (int i = 0; i< m1->rows;i++){
        for (int j=0; j< m1->cols;j++){
            double d1 = m1->data[i*max_cols + j];
            double d2 = m2->data[i*max_cols + j];
            result->data[i*max_cols + j] = d1 - d2;
        }
    }

    return result;
}

Matrix *mult_matrix(Matrix *m1, Matrix *m2){
//Multiplication de deux matrices

    if (m1->cols != m2->rows){
        return NULL;
    }
    Matrix *result = create_matrix(m1->rows,m2->cols);
    int max_cols = result->cols;

    for (int i = 0; i< result->rows;i++){
        for (int j=0; j< result->cols;j++){
            double sum = 0;
            for (int k = 0; k < m1->cols;k++){
                sum =sum + m1->data[i*m1->cols + k]* m2->data[k*m2->cols + j];
            }
            result->data[i*max_cols + j] = sum;
        }
    }
    return result;
}

Matrix *scalar_mult_matrix(Matrix *m, double scalar){
//multiplie la matrice par un scalaire

    Matrix *result = create_matrix(m->rows, m->cols);
    for (int i = 0; i< result->rows;i++){
        for (int j=0; j< result->cols;j++){
            result->data[i*m->cols + j] = m->data[i*m->cols + j] * scalar;
        }
    }
    return result;
}

Matrix *mult_by_ele_matrix(Matrix *m1, Matrix *m2){
//Multiplie éléments par éléments -> Doivent avoir même dimensions
 
    if (m1->rows != m2->rows || m1->cols != m2->cols){
        return NULL;
    }
    Matrix *result = create_matrix(m1->rows,m2->cols);
    int max_cols = m1->cols;
 
    for (int i = 0; i< m1->rows;i++){
        for (int j=0; j< m1->cols;j++){
            double d1 = m1->data[i*max_cols +j];
            double d2 = m2->data[i*max_cols +j];
            result->data[i*max_cols + j] = d1 * d2;
        }
    }
    return result;
}

Matrix *fct_matrix(Matrix *m1, double (*fct)(double)){
//Applique une fonction à tous les éléments d'une matrice
 
    Matrix *result = create_matrix(m1->rows,m1->cols);
    int max_cols = m1->cols;
 
    for (int i = 0; i< m1->rows;i++){
        for (int j=0; j< m1->cols;j++){
            result->data[i*max_cols + j] = fct(m1->data[i*max_cols + j]);
        }
    }
    return result;
}

Matrix *transpose_matrix(Matrix *m){
//Calcule la transposé de la matrice

    Matrix *result = create_matrix(m->cols, m->rows);
    for (int i = 0; i < m->rows; i++){
        for (int j = 0; j < m->cols; j++){
            result->data[j*m->rows + i] = m->data[i*m->cols + j];
        }
    }
    return result;
}

Matrix *sum_rows(Matrix *m){
    //calcule la somme des lignes d'une matrice ex : 3x6 -> 1x6

    Matrix *result = create_matrix(1, m->cols);
    for (int j = 0; j < m->cols; j++)
        for (int i = 0; i < m->rows; i++)
            result->data[j] += m->data[i*m->cols + j];
    return result;
}
