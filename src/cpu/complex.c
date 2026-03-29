/* src/cpu/complex.c */

#include "complex.h"
#include <stdio.h>

/* (a + ib) + (c + id) = (a + c) + i(b + d) */
Complex complex_add(Complex a, Complex b)
{
    Complex sum = {
        a.real + b.real,
        a.imaginary + b.imaginary
    };

    return sum;
}

/* (a + ib) * (c + id) = (ac - bd) + i(ad + bc) */
Complex complex_mul(Complex a, Complex b)
{
    Complex prod = {
        (a.real * b.real) - (a.imaginary * b.imaginary),
        (a.real * b.imaginary + a.imaginary * b.real)
    };

    return prod;
}

/* conjugate: (real, -imaginary) */
Complex complex_conj(Complex a)
{
    Complex conj = {
        a.real,
        0 - a.imaginary
    };

    return conj;
}

/* |a|^2 = (real)^2 + (imaginary)^2 */
float complex_abs2(Complex a)
{
    return a.real * a.real + a.imaginary * a.imaginary;
}

void complex_print(Complex c)
{
    printf("(%lf + i%lf)", c.real, c.imaginary);
}
