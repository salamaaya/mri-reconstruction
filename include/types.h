/* include/types.h */

/* COMPLEX */
typedef struct {
    float real;
    float imaginary;
} Complex;

Complex complex_add(Complex a, Complex b);
Complex complex_mul(Complex a, Complex b);
Complex complex_conj(Complex a);
float   complex_abs2(Complex a);

/* KSPACE */
typedef struct {
    int slices;
    int rows;
    int cols;
    Complex *data;
} KSpace;
