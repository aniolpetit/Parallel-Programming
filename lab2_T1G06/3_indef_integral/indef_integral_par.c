#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

double function(double x) {
    return x * sin(x);
}

double exact_integral(double x) {
    return sin(x) - x * cos(x);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check parameters
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

    int local_N = N / size;
    int extra_N = N % size;

    int start_i = rank * local_N + (rank < extra_N ? rank : extra_N);
    int end_i = start_i + local_N + (rank < extra_N ? 1 : 0);

    // Compute the number of elements each process will handle
    int local_size = end_i - start_i + 1;

    // Allocate memory for local integral array
    double* local_integral = (double *)calloc(local_size,sizeof(double));
    
    clock_t start_time = clock();
    // Compute local integral values
    double local_sum = 0.0;
    for (int i = 1; i < local_size; i++) {
        double x = (start_i + i - 0.5) * deltaX;
        local_integral[i] = local_integral[i-1] + deltaX*function(x);
    }

    // Gather local integral arrays to process 0
    double* integral = NULL;
    if (rank == 0) {
        integral = (double *)malloc(size * sizeof(double));
    }
x
    MPI_Gather(&local_integral[local_size - 1], 1, MPI_DOUBLE, integral, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Print results on process 0
    if (rank == 0) {
        double def_result = 0.0;
        for(int i = 0; i<size; i++){
            def_result += integral[i];
        }
        
        double exact = exact_integral(X_MAX);
        double error = fabs(def_result - exact);
        double run_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        printf("Result with N=%d is %.12lf (%.12lf, %.2e) in %lf seconds\n", N, def_result, exact, error, run_time );

        // Print integral data to file
        char filename_dat[100];
        sprintf(filename_dat, "indef_integral_mpi.dat");
        FILE* file_dat = fopen(filename_dat, "wb");
        if (file_dat != NULL) {
            fwrite(integral, sizeof(double), N, file_dat);
            fclose(file_dat);
        }

        // Print integral info to file
        char filename_info[100];
        sprintf(filename_info, "indef_integral_mpi.info");
        FILE* file_info = fopen(filename_info, "w");
        if (file_info != NULL) {
            fprintf(file_info, "# %s\n", "Indefinite integral version MPI");
            fprintf(file_info, "data %s\n", filename_dat);
            fprintf(file_info, "N %d\n", N);
            fprintf(file_info, "XMAX %lf\n", X_MAX);
            fprintf(file_info, "size %d\n", size);
            fclose(file_info);
        }

        free(integral);
    }

    free(local_integral);
    MPI_Finalize();
    return 0;
}
