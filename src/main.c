/* main.c */

#include <stdio.h>

#include "types.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    Complex a = {3, 2};
    Complex b = {3, -2};

    Complex sum = complex_add(a, b);
    Complex prod = complex_mul(a, b);
    Complex conj = complex_conj(a);
    float abs2 = complex_abs2(a);

    printf("Added (%lf + %lfi) + (%lf + %lfi) = (%lf + %lfi)\n",
            a.real, a.imaginary, b.real, b.imaginary,
            sum.real, sum.imaginary);

    printf("Multiplied (%lf + %lfi) * (%lf + %lfi) = (%lf + %lfi)\n",
            a.real, a.imaginary, b.real, b.imaginary,
            prod.real, prod.imaginary);

    printf("Complex conjugate (%lf + %lfi) = (%lf + %lfi)\n",
            a.real, a.imaginary, conj.real, conj.imaginary);

    printf("Complex abs squared (%lf + %lfi) = %lf\n",
            a.real, a.imaginary, abs2);

    return 0;
}
