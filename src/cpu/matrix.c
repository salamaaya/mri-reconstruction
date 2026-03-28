/* src/cpu/matrix.c */

#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

/* caller must free malloced memory */
Matrix* matrix_create(int rows, int cols)
{
    if (rows < 0 || cols < 0) {
        return NULL;
    }

    int i, j;
    int bytes = rows * cols * sizeof(float);
    float *data;
    Matrix *matrix;

    data = (float *)malloc(bytes);
    if (data == NULL) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        return NULL;
    }
    matrix = (Matrix *)malloc(sizeof(Matrix));
    if (matrix == NULL) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        free(data);
        return NULL;
    }

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = data;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            matrix_setat(matrix, 0, i, j);
    }

    return matrix;
}

void matrix_destroy(Matrix *m)
{
    free(m->data);
    free(m);
}

float matrix_getat(Matrix *m, int i, int j)
{
    if (i > m->rows || i < 0) return -1; 
    if (j > m->cols || j < 0) return -1; 

    return m->data[i * m->rows + j];
}

int matrix_setat(Matrix *m, float val, int i, int j)
{
    if (i > m->rows || i < 0) return -1; 
    if (j > m->cols || j < 0) return -1; 

    m->data[i * m->rows + j] = val;
    return 0;
}

/* caller must free malloced memory */
Matrix* matrix_add(Matrix *a, Matrix *b)
{
    if (a->rows != b->rows && a->cols != b->cols) {
        return NULL;
    }

    int i, j;
    float s;

    Matrix *sum = (Matrix *)malloc(sizeof(Matrix));
    float *data = (float *)malloc(a->rows * a->cols * sizeof(float));

    sum->rows = a->rows;
    sum->cols = a->cols;
    sum->data = data;

    for (i = 0; i < a->rows; i++) {
        for (j = 0; j < a->cols; j++) {
            s = matrix_getat(a, i, j) + matrix_getat(b, i, j);
            matrix_setat(sum, s, i, j);
        }
    }

    return sum;
}

Matrix* matrix_mul(Matrix, Matrix);
Matrix* matrix_transpose(Matrix, Matrix);
Matrix* matrix_hermitian(Matrix, Matrix);

