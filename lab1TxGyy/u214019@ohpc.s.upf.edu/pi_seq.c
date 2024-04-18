#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]){
    if(argc<2){
        printf("N must be passed as a parameter!");
        exit(0);    
    }
    int N = atoi(argv[1]);
    float pi = 0;
    float width = (float) 1/N;
    float fxi = 0;
    float xi = 0;
    float start_time = omp_get_wtime();
    for(int i = 0; i < N; i++){
        xi = (i+0.5)*width;
        fxi = 4/(1+xi*xi);
        pi+=fxi;
    }
    pi = pi*width;
    float end_time = omp_get_wtime();
    float runtime = end_time - start_time;

    printf("\n Pi with %d steps is %.15lf in %lf seconds", N, pi, runtime);
}