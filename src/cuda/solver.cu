/* src/gpu/solver.cu */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nufft.h"
#include "solver.h"
#include "vector.h"

static int apply_normal_operator(const Complex *x,
                                 const float *kx,
                                 const float *ky,
                                 int m,
                                 int nx,
                                 int ny,
                                 float lambda,
                                 Complex *tmp_samples,
                                 Complex *out)
{
    const int n = nx * ny;

    nufft(x, nx, ny, kx, ky, m, tmp_samples, out);

    for (int i = 0; i < n; i++) {
        out[i].real += lambda * x[i].real;
        out[i].imaginary += lambda * x[i].imaginary;
    }

    return 0;
}

/*
 * Reconstruct one image slice by solving:
 * (F^H F + lambda I) rho = F^H d
 * with Conjugate Gradient on complex vectors.
 *
 * Returns the number of iterations used, or -1 on failure.
 */
int cg_reconstruct_slice(const Complex *samples,
                         const float *kx,
                         const float *ky,
                         int m,
                         int nx,
                         int ny,
                         float lambda,
                         int max_iter,
                         float tol,
                         int verbose,
                         Complex *rho_out)
{
    if (!samples || !kx || !ky || !rho_out || m <= 0 || nx <= 0 || ny <= 0 || max_iter <= 0) {
        return -1;
    }

    const int n = nx * ny;
    Complex *b = (Complex *)malloc((size_t)n * sizeof(Complex));
    Complex *r = (Complex *)malloc((size_t)n * sizeof(Complex));
    Complex *p = (Complex *)malloc((size_t)n * sizeof(Complex));
    Complex *ap = (Complex *)malloc((size_t)n * sizeof(Complex));
    Complex *tmp_samples = (Complex *)malloc((size_t)m * sizeof(Complex));

    if (!b || !r || !p || !ap || !tmp_samples) {
        free(b);
        free(r);
        free(p);
        free(ap);
        free(tmp_samples);
        return -1;
    }

    /* b = F^H d */
    nufft_adjoint(samples, m, kx, ky, nx, ny, b);

    /* x0 = 0, r0 = b - A*x0 = b, p0 = r0 */
    vec_zero(rho_out, n);
    vec_copy(r, b, n);
    vec_copy(p, r, n);

    const float b_norm = sqrtf(vec_norm2(b, n)) + 1e-12f;
    float r_norm = sqrtf(vec_norm2(r, n));

    if (verbose) {
        printf("CG start: ||r0||/||b|| = %.6e\n", r_norm / b_norm);
    }

    int it;
    for (it = 0; it < max_iter; it++) {
        Complex rr = vec_dot_hermitian(r, r, n);

        apply_normal_operator(p, kx, ky, m, nx, ny, lambda, tmp_samples, ap);
        Complex p_ap = vec_dot_hermitian(p, ap, n);

        const float denom = p_ap.real;
        if (fabsf(denom) < 1e-20f) {
            break;
        }

        Complex alpha = {
            rr.real / denom,
            rr.imaginary / denom};

        vec_axpy_complex(rho_out, alpha, p, n);

        Complex minus_alpha = {-alpha.real, -alpha.imaginary};
        vec_axpy_complex(r, minus_alpha, ap, n);

        r_norm = sqrtf(vec_norm2(r, n));
        if (verbose && ((it + 1) % 5 == 0 || it == 0)) {
            printf("CG iter %d: ||r||/||b|| = %.6e\n", it + 1, r_norm / b_norm);
        }
        if (r_norm / b_norm < tol) {
            it += 1;
            break;
        }

        Complex rr_new = vec_dot_hermitian(r, r, n);
        Complex beta = {
            rr_new.real / (rr.real + 1e-20f),
            rr_new.imaginary / (rr.real + 1e-20f)};

        vec_scale_add(p, r, beta, p, n);
    }

    free(b);
    free(r);
    free(p);
    free(ap);
    free(tmp_samples);

    return it;
}

