/* include/matrix.h */

typedef struct {
    int rows;
    int cols;
    float *data;
} Matrix;

Matrix* matrix_create(int, int);
void    matrix_destroy(Matrix *);

float   matrix_getat(Matrix *, int, int);
int     matrix_setat(Matrix *, float, int, int);

Matrix* matrix_add(Matrix *, Matrix *);
Matrix* matrix_mul(Matrix, Matrix);
Matrix* matrix_transpose(Matrix, Matrix);
Matrix* matrix_hermitian(Matrix, Matrix);

