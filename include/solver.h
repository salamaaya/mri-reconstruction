#ifndef SOLVER_H
#define SOLVER_H

#include "complex.h"

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
                         Complex *rho_out);

#endif /* SOLVER_H */
