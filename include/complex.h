/* include/complex.h */

typedef struct {
    float real;
    float imaginary;
} Complex;

Complex complex_add(Complex a, Complex b);
Complex complex_mul(Complex a, Complex b);
Complex complex_conj(Complex a);
float   complex_abs2(Complex a);

