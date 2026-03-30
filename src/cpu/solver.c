#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nufft.h"
#include "solver.h"

static void vec_zero(Complex *v, int n)
{
    memset(v, 0, (size_t)n * sizeof(Complex));
}

static void vec_copy(Complex *dst, const Complex *src, int n)
{
    memcpy(dst, src, (size_t)n * sizeof(Complex));
}

static void vec_axpy_complex(Complex *y, Complex alpha, const Complex *x, int n)
{
    for (int i = 0; i < n; i++)
    {
        const float xr = x[i].real;
        const float xi = x[i].imaginary;
        y[i].real += alpha.real * xr - alpha.imaginary * xi;
        y[i].imaginary += alpha.real * xi + alpha.imaginary * xr;
    }
}

static void vec_scale_add(Complex *out, const Complex *a, Complex beta, const Complex *b, int n)
{
    for (int i = 0; i < n; i++)
    {
        const float br = b[i].real;
        const float bi = b[i].imaginary;
        out[i].real = a[i].real + beta.real * br - beta.imaginary * bi;
        out[i].imaginary = a[i].imaginary + beta.real * bi + beta.imaginary * br;
    }
}

/* Returns sum(conj(a_i) * b_i). */
static Complex vec_dot_hermitian(const Complex *a, const Complex *b, int n)
{
    Complex dot = {0.0f, 0.0f};
    for (int i = 0; i < n; i++)
    {
        dot.real += a[i].real * b[i].real + a[i].imaginary * b[i].imaginary;
        dot.imaginary += a[i].real * b[i].imaginary - a[i].imaginary * b[i].real;
    }
    return dot;
}

static float vec_norm2(const Complex *v, int n)
{
    float norm2 = 0.0f;
    for (int i = 0; i < n; i++)
    {
        norm2 += v[i].real * v[i].real + v[i].imaginary * v[i].imaginary;
    }
    return norm2;
}

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

    nufft_forward(x, nx, ny, kx, ky, m, tmp_samples);
    nufft_adjoint(tmp_samples, m, kx, ky, nx, ny, out);

    for (int i = 0; i < n; i++)
    {
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
    for (it = 0; it < max_iter; it++)
    {
        Complex rr = vec_dot_hermitian(r, r, n);

        apply_normal_operator(p, kx, ky, m, nx, ny, lambda, tmp_samples, ap);
        Complex p_ap = vec_dot_hermitian(p, ap, n);

        const float denom = p_ap.real;
        if (fabsf(denom) < 1e-20f)
        {
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
