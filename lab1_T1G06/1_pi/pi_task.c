#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


double compute_pi(int start, int end, double width) {
    double partial_pi = 0;
    double xi, fxi;
    for (int i = start; i < end; i++) {
        xi = (i + 0.5) * width;
        fxi = 4 / (1 + xi * xi);
        partial_pi += fxi;
    }
    return partial_pi;
}

void compute_pi_task(int start, int end, double width, double *result, int M) {
    if (end - start <= M) {
        *result = compute_pi(start, end, width);
    } 
    else {
        int mid = (start + end) / 2;
        double partial_pi1, partial_pi2;
        #pragma omp task shared(partial_pi1) 
                compute_pi_task(start, mid, width, &partial_pi1, M);
        #pragma omp task shared(partial_pi2) 
                compute_pi_task(mid, end, width, &partial_pi2, M);
        #pragma omp taskwait
        *result = partial_pi1 + partial_pi2;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("N and M must be passed as parameters!");
        exit(1);
    }
    
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    
    if (M < 1024) {
        printf("Each task must perform at least 1024 steps (M>=1024) ! \n");
        exit(1);
    }

    double pi = 0;
    double width = 1.0 / N;
    double start_time = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
            compute_pi_task(0, N, width, &pi, M);
        }        
    }
    pi *= width;
    double end_time = omp_get_wtime();
    double runtime = end_time - start_time;

    printf("\nPi with %d steps is %.15lf in %lf seconds", N, pi, runtime);

    return 0;
}
