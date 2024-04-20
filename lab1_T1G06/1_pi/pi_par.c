#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("N must be passed as a parameter!");
        exit(0);    
    }

    int N = atoi(argv[1]);
    omp_set_num_threads(8);
    double pi = 0.0;
    int nthreads = omp_get_max_threads();
    double *local_sum = (double *)malloc(nthreads * sizeof(double));
    double start_time, end_time;
    double width = (double)1/N;

    start_time = omp_get_wtime();

    // Parallel region starts here
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        local_sum[id] = 0.0;

        for(int i = id; i < N; i += nthreads) {
            double xi = (i + 0.5)*width;
            local_sum[id] += 4.0 / (1.0 + xi * xi);
        }
    }
    // Parallel region ends here
    // Summing up the local sums from each thread
    for(int i = 0; i < nthreads; i++) {
        pi += local_sum[i];
    }

    pi = pi*width;
    end_time = omp_get_wtime();
    printf("\nPi with %d steps is %.15lf in %lf seconds\n", N, pi, end_time - start_time);

    free(local_sum);
    return 0;
}
