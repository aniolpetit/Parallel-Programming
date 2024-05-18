#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 1024
#define ROWSIZE 9

void spmv_cpu(int m, int r, double* vals, int* cols, double* x, double* y) {
    for (int i = 0; i < m; i++) {
        double partial_sum = 0.0;
        for (int j = 0; j < r; j++) {
            int idx = i * r + j;
            partial_sum += vals[idx] * x[cols[idx]];
        }
        y[i] = partial_sum;
    }
}

void spmv_gpu(int m, int r, double* vals, int* cols, double* x, double* y) {
    #pragma acc parallel loop gang vector present(vals[:m * r], cols[:m * r], x[:m], y[:m])
    for (int i = 0; i < m; i++) {
        double partial_sum = 0.0;
        #pragma acc loop reduction(+:partial_sum)
        for (int j = 0; j < r; j++) {
            int idx = i * r + j;
            partial_sum += vals[idx] * x[cols[idx]];
        }
        y[i] = partial_sum;
    }
}

void axpy_gpu(int n, double alpha, double* x, double* y) {
    #pragma acc parallel loop present(x[:n], y[:n])
    for (int i = 0; i < n; i++) {
        y[i] += alpha * x[i];
    }
}

double dot_product_gpu(int n, double* x, double* y){
    double result = 0.0;
    #pragma acc parallel loop present(x[:n], y[:n]) reduction(+:result) //implicit copy
    for(int i = 0; i < n; i++){
        result += x[i]*y[i];
    }
    return result;
}

void fill_matrix(double* vals, int* cols) {
    int indx[ROWSIZE];
    int row_count = 0;
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            indx[0] = i + (j - 2) * N;
            indx[1] = i + (j - 1) * N;
            indx[2] = i - 2 + j * N;
            indx[3] = i - 1 + j * N;
            indx[4] = i + j * N;
            indx[5] = i + 1 + j * N;
            indx[6] = i + 2 + j * N;
            indx[7] = i + (j + 1) * N;
            indx[8] = i + (j + 2) * N;

            for (int row = 0; row < ROWSIZE; row++) {
                if (indx[row] < 0 || indx[row] >= N * N) {
                    cols[row + row_count * ROWSIZE] = i + j * N;
                    vals[row + row_count * ROWSIZE] = 0.0;
                } else {
                    cols[row + row_count * ROWSIZE] = indx[row];
                    if (row == 4) {
                        vals[row + row_count * ROWSIZE] = 0.95;
                    } else {
                        vals[row + row_count * ROWSIZE] = -0.95 / (ROWSIZE - 1);
                    }
                }
            }
            row_count++;
        }
    }
    vals[4 + (N * N / 2 + N / 2) * ROWSIZE] = 1.001 * vals[4 + (N * N / 2 + N / 2) * ROWSIZE];
}

void create_solution_and_rhs(int vecsize, double* Avals, int* Acols, double* xsol, double* rhs) {
    for (int i = 0; i < vecsize; i++)
        xsol[i] = sin(i * 0.1) + cos(i * 0.01);

    spmv_cpu(vecsize, ROWSIZE, Avals, Acols, xsol, rhs);
}

void cg_gpu(int vec_size, double* Avals, int* Acols, double* rhs, double* x) {
    int iterations = 500;
    double alpha, beta, rho1, rho0, denom;
    double* Ax = (double*)malloc(vec_size * sizeof(double));
    double* r0 = (double*)malloc(vec_size * sizeof(double));
    double* p0 = (double*)malloc(vec_size * sizeof(double));

    #pragma acc enter data create(Ax[:vec_size], r0[:vec_size], p0[:vec_size])

    #pragma acc parallel loop present(r0[:vec_size], rhs[:vec_size])
    for (int i = 0; i < vec_size; i++) {
        r0[i] = rhs[i];
    }

    spmv_gpu(vec_size, ROWSIZE, Avals, Acols, x, Ax);
    axpy_gpu(vec_size, -1.0, Ax, r0);

    #pragma acc parallel loop present(p0[:vec_size], r0[:vec_size])
    for (int i = 0; i < vec_size; i++) {
        p0[i] = r0[i];
    }

    for (int k = 0; k < iterations; k++) {
        spmv_gpu(vec_size, ROWSIZE, Avals, Acols, p0, Ax);
        rho0 = dot_product_gpu(vec_size, r0, r0);
        denom = dot_product_gpu(vec_size, p0, Ax);

        alpha = rho0 / denom;

        axpy_gpu(vec_size, alpha, p0, x);

        axpy_gpu(vec_size, -1.0 * alpha, Ax, r0);

        rho1 = dot_product_gpu(vec_size, r0, r0);

        if (k % 20 == 0)
            printf("Iteration %d, residual %e\n", k, rho1);

        beta = rho1 / rho0;
        #pragma acc parallel loop present(p0[:vec_size], r0[:vec_size])
        for (int i = 0; i < vec_size; i++)
            p0[i] = r0[i] + beta * p0[i];
    }

    #pragma acc exit data delete(Ax[:vec_size], r0[:vec_size], p0[:vec_size])

    free(Ax);
    free(r0);
    free(p0);
}

int main() {
    int vec_size = N * N;
    double time_start, time_gpu;

    double* x_gpu = (double*)malloc(vec_size * sizeof(double));
    double* x_sol = (double*)malloc(vec_size * sizeof(double));
    double* rhs = (double*)malloc(vec_size * sizeof(double));

    double* Avals = (double*)malloc(ROWSIZE * vec_size * sizeof(double));
    int* Acols = (int*)malloc(ROWSIZE * vec_size * sizeof(int));

    for (int i = 0; i < vec_size; i++) {
        x_gpu[i] = 0.0;
        x_sol[i] = 0.0;
        rhs[i] = 0.0;
    }

    // Define a problem to test the cg code
    fill_matrix(Avals, Acols);
    create_solution_and_rhs(vec_size, Avals, Acols, x_sol, rhs);

    // Copy necessary data to the device
    #pragma acc enter data copyin(Avals[:vec_size * ROWSIZE], Acols[:vec_size * ROWSIZE], rhs[:vec_size], x_sol[:vec_size], x_gpu[:vec_size])

    time_start = omp_get_wtime();

    cg_gpu(vec_size, Avals, Acols, rhs, x_gpu);

    time_gpu = omp_get_wtime() - time_start;

    // Copy the result back to the host
    #pragma acc exit data copyout(x_gpu[:vec_size], x_sol[:vec_size])

    // Compare GPU solution with real solution
    double norm2 = 0.0;
    for (int i = 0; i < vec_size; i++)
        norm2 += (x_gpu[i] - x_sol[i]) * (x_gpu[i] - x_sol[i]);

    printf("cg error in gpu solution: %e, size %d\n", sqrt(norm2), vec_size);

    printf("Time GPU: %lf\n", time_gpu);

    free(rhs);
    free(x_gpu);
    free(x_sol);
    free(Avals);
    free(Acols);
}
