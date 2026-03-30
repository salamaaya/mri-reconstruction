/* include/complex.h */

#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct {
    float real;
    float imaginary;
} Complex;

Complex complex_add(Complex, Complex);
Complex complex_mul(Complex, Complex);
Complex complex_conj(Complex);
float   complex_abs2(Complex);

void    complex_print(Complex);

#endif /* COMPLEX_H */
