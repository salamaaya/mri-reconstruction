/* src/cpu/matrix.c */

#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "complex.h"

/* 
 * caller must free malloced memory
 */
Matrix* matrix_construct(int rows, int cols)
{
    if (rows < 0 || cols < 0) {
        return NULL;
    }

    int i, j;
    int bytes = rows * cols * sizeof(Complex);
    Matrix *matrix;

    matrix = (Matrix *)malloc(sizeof(Matrix));
    if (!matrix) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        return NULL;
    }

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = (Complex *)malloc(bytes);
    if (!matrix->data) {
        fprintf(stderr, "malloc failed (out of memory?).\n");
        return NULL;
    }

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            matrix_setat(matrix, (Complex){0, 0}, i, j);
    }

    return matrix;
}

void matrix_destroy(Matrix *m)
{
    if (m->data) free(m->data);
    free(m);
}

Complex matrix_getat(Matrix *m, int i, int j)
{
    if (i >= m->rows || i < 0) return (Complex){0, 0}; 
    if (j >= m->cols || j < 0) return (Complex){0, 0} ;

    return m->data[i * m->cols + j];
}

int matrix_setat(Matrix *m, Complex val, int i, int j)
{
    if (i >= m->rows || i < 0) return -1; 
    if (j >= m->cols || j < 0) return -1; 

    m->data[i * m->cols + j] = val;
    return 0;
}

/* 
 * caller must free malloced memory
 */
Matrix* matrix_add(Matrix *a, Matrix *b)
{
    if (a->rows != b->rows || a->cols != b->cols) {
        return NULL;
    }

    int i, j;
    Complex s;

    Matrix *sum = matrix_construct(a->rows, a->cols);

    for (i = 0; i < a->rows; i++) {
        for (j = 0; j < a->cols; j++) {
            s = complex_add(matrix_getat(a, i, j), matrix_getat(b, i, j));
            matrix_setat(sum, s, i, j);
        }
    }

    return sum;
}

/* 
 * caller must free malloced memory
 * https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm
 */
Matrix* matrix_mul(Matrix *a, Matrix *b)
{
    if (a->cols != b->rows) {
        return NULL;
    }

    int i, j, k;
    Complex sum, prod;

    Matrix *product = matrix_construct(a->rows, b->cols);

    for (i = 0; i < a->rows; i++) {
        for (j = 0; j < b->cols; j++) {
            sum = (Complex){0, 0};
            for (k = 0; k < b->rows; k++) {
                prod = complex_mul(matrix_getat(a, i, k), matrix_getat(b, k, j));
                sum = complex_add(sum, prod);
            }
            matrix_setat(product, sum, i, j);
        }
    }

    return product;
}

/* 
 * caller must free malloced memory
 */
Matrix* matrix_transpose(Matrix *m)
{
    int i, j;
    Complex val;
    Matrix *transpose = matrix_construct(m->cols, m->rows);

    for (i = 0; i < m->rows; i++) {
        for (j = 0; j < m->cols; j++) {
            val = matrix_getat(m, i, j);
            matrix_setat(transpose, val, j, i);
        }
    }

    return transpose;
}

/* 
 * caller must free malloced memory
 * M = M^H => hermition matrix
 */
Matrix* matrix_hermitian(Matrix *m)
{
    int i, j;
    Complex val;
    Matrix *hermition = matrix_construct(m->cols, m->rows);

    for (i = 0; i < m->rows; i++) {
        for (j = 0; j < m->cols; j++) {
            val = matrix_getat(m, i, j);
            val = complex_conj(val);
            matrix_setat(hermition, val, j, i);
        }
    }

    return hermition;
}

void matrix_print(Matrix *m)
{
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            complex_print(matrix_getat(m, i, j));
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}
