#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

double function(double x) {
    return x * sin(x);
}

double exact_quadrature(double x) {
    return sin(x) - x * cos(x);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            fprintf(stderr, "Wrong number of parameters\nUsage:\n\t%s N XMAX\n", argv[0]);
        }
        MPI_Finalize();
        return -1;
    }

    int N = strtol(argv[1], NULL, 10);
    double X_MAX = strtod(argv[2], NULL);
    double deltaX = X_MAX / (double)N;

    // Compute the number of subintervals each process will handle
    int local_N = N / size;
    int extra_N = N % size;

    // Compute the starting and ending index of subintervals for each process
    int start_i = rank * local_N + (rank < extra_N ? rank : extra_N);
    int end_i = start_i + local_N + (rank < extra_N ? 1 : 0);

    // Initialize local result for each process
    double local_result = 0;

    // Compute partial sum for each process
    double start_time = MPI_Wtime();
    int i;
    for (i = start_i + 1; i <= end_i; i++) {
        double x_middle = (i - 0.5) * deltaX;
        local_result += function(x_middle) * deltaX;
    }

    // Gather local results from all processes
    double global_result;
    MPI_Reduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    // Compute error and print results
    if (rank == 0) {
        double exact = exact_quadrature(X_MAX);
        double error = fabs(global_result - exact);
        printf("Result with N=%d is %.12lf (%.12lf, %.2e) in %lf seconds\n", N, global_result, exact, error, end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
