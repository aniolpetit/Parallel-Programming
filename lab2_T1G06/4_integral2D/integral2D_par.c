#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define X_MAX 1.0
#define Y_MAX 1.0

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv); // Initialize MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //printf("Integral 2D: Rank %d of %d\n", rank, size);

    /// Check parameters
    if(argc != 4) {
        if(rank == 0)
            fprintf(stderr, "Wrong number of parameters\nUsage:\n\t%s filename Nx Ny\n", argv[0]);
        MPI_Finalize();
        return -1;
    }
    char* filename = argv[1];
    int Nx = (int)strtol(argv[2], NULL, 10);
    int Ny = (int)strtol(argv[3], NULL, 10);

    const double deltaX = 2.0*X_MAX/(double) Nx;
    const double deltaY = 2.0*Y_MAX/(double) Ny;

    int rows_per_proc = Ny / size;
    int extra_rows = Ny % size;

    int start_row = rank * rows_per_proc + (rank < extra_rows ? rank : extra_rows);
    int end_row = start_row + rows_per_proc + (rank < extra_rows ? 1 : 0);

    // Read data
    double* data = (double*)malloc((end_row - start_row) * Nx * sizeof(double));
    MPI_File file;
    MPI_Offset offset = start_row * Nx * sizeof(double);
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read_at(file, offset, data, (end_row - start_row) * Nx, MPI_DOUBLE, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    double start_time = omp_get_wtime();
    // Compute integral
    double result = 0.0;
    #pragma omp parallel for collapse(2) reduction(+:result)
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < Nx; j++) {
            // double x = -X_MAX + (j + 0.5) * deltaX; 
            // double y = -Y_MAX + (i + 0.5) * deltaY;
            result += data[(i - start_row) * Nx + j] * deltaX * deltaY;
        }
    }
    
    double run_time = omp_get_wtime() - start_time;

    //  Print results
    if(rank == 0)
    {
        printf("Result with N=%d M=%d size: %d threads: %d is %.12lf in %lf seconds\n", Nx, Ny,
			size, omp_get_max_threads(), result, run_time);
    }

    free(data);
    MPI_Finalize();
    return 0;
}
