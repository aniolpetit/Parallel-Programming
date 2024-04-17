#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

//IF N IS GREAT (1024^3 FOR INSTANCE) I DON'T GET A CORRECT RESULT, ACTUALLY NOTHING. WHY?
int main(int argc, char* argv[]) {
    if(argc < 2) {
        printf("N must be passed as a parameter!");
        exit(0);    
    }

    int N = atoi(argv[1]);
    omp_set_num_threads(4);
    float pi = 0;
    float width = (float) 1/N;
    float xi = 0;
    float start_time = omp_get_wtime();
    int nthreads = omp_get_num_threads();
    float *local_sum = (float *)malloc(nthreads * sizeof(float));

    // Parallel region starts here
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
       
        int first = N/nthreads*id;
        int last = N/nthreads*(id+1);

        if(last > N){
            last = N;
        }
        
        local_sum[id] = 0.0f;
        for(int i = first; i < last; i++) {
            xi = (i + 0.5) * width;
            local_sum[id] += 4 / (1 + xi * xi);
        }
    }
    // Parallel region ends here
    for(int i = 0; i < nthreads; i++) {
        pi += local_sum[i];
    }

    pi *= width;
    float end_time = omp_get_wtime();
    float runtime = end_time - start_time;

    printf("\n Pi with %d steps is %.15lf in %lf seconds\n", N, pi, runtime);

    return 0;
}