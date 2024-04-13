#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("N and M must be passed as parameters!");
        exit(0);
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]); // Minimum number of steps in a task
    float pi = 0;
    float width = 1.0f / N;
    float start_time = omp_get_wtime();

    #pragma omp parallel
    #pragma omp single
    {
        int num_tasks = (N + M - 1) / M; // Calculate number of tasks needed

        // Create tasks for each chunk of work
        for (int i = 0; i < num_tasks; i++) {
            int start = i * M;
            int end = (i == num_tasks - 1) ? N : start + M;

            #pragma omp task shared(pi)
            {
                float partial_pi = 0.0f;
                for (int j = start; j < end; j++) {
                    float xi = (j + 0.5) * width;
                    partial_pi += 4 / (1 + xi * xi);
                }
                #pragma omp atomic
                pi += partial_pi;
            }
        }
    }

    pi *= width;
    float end_time = omp_get_wtime();
    float runtime = end_time - start_time;

    printf("\n Pi with %d steps is %.15lf in %lf seconds", N, pi, runtime);

    return 0;
}
