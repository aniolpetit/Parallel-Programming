#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main(int argc, char* argv[]){
    if(argc<2){
        printf("N must be passed as a parameter!");
        exit(0);    
    }
    int N = atoi(argv[1]);
    double pi = 0;
    double width = (double) 1/N;
    double fxi = 0;
    double xi = 0;
    double start_time = omp_get_wtime();
    for(int i = 0; i < N; i++){
        xi = (i+0.5)*width;
        fxi = 4/(1+xi*xi);
        pi+=fxi;
    }
    pi = pi*width;
    double end_time = omp_get_wtime();
    double runtime = end_time - start_time;

    printf("\nPi with %d steps is %.15lf in %lf seconds", N, pi, runtime);
}