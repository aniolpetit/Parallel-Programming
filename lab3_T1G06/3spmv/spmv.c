#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<omp.h>

#define N 1024    // number of matrix rows will be N*N
#define ROWSIZE 9 // number of nonzero cols of sparse matrix

void spmv_cpu(int m, int r, double* vals, int* cols, double* x, double* y){
    for(int i = 0; i < m; i++){
        double partial_sum = 0.0;
        for(int j = 0; j < r; j++){
            int idx = i*r + j;
            partial_sum += vals[idx]*x[cols[idx]];
        }
        y[i] = partial_sum;
    }
}

void spmv_gpu(int m, int r, double *vals, int *cols, double *x, double *y){
    #pragma acc parallel loop gang vector present(vals[:m*r], cols[:m*r], x[:m], y[:m])
    for (int i = 0; i < m; i++)
    {
        double partial_sum = 0.0;
        #pragma acc loop reduction(+:partial_sum) 
        for (int j = 0; j < r; j++)
        {
            int idx = i*r + j;
            partial_sum += vals[idx]*x[cols[idx]];
        }
        y[i] = partial_sum;
    }
}


void fill_matrix(double* vals, int* cols)
{

    int indx[ROWSIZE];
    int row_count = 0;
    for(int j = 0; j < N ; j++){
        for(int i = 0; i < N; i++){

            indx[0] = i     + (j - 2)*N;
            indx[1] = i     + (j - 1)*N;
            indx[2] = i - 2 +  j     *N;
            indx[3] = i - 1 +  j     *N;
            indx[4] = i     +  j     *N;
            indx[5] = i + 1 +  j     *N;
            indx[6] = i + 2 +  j     *N;
            indx[7] = i     + (j + 1)*N;
            indx[8] = i     + (j + 2)*N;

            for(int row = 0; row < ROWSIZE; row++)
            {
                if(indx[row] < 0 || indx[row] >= N*N)
                {
                    cols[row + row_count*ROWSIZE] = i + j*N;
                    vals[row + row_count*ROWSIZE] = 0.0;
                }
                else
                {
                    cols[row + row_count*ROWSIZE] = indx[row];
                    if(row == 4)
                    {
                        vals[row + row_count*ROWSIZE] = 0.95;
                    }
                    else
                    {
                        vals[row + row_count*ROWSIZE] = -0.95/(ROWSIZE - 1);
                    }
                }
            }
            row_count++;
        }
    }

    vals[4 + (N*N/2 + N/2)*ROWSIZE] =  1.001*vals[4 + (N*N/2 + N/2)*ROWSIZE];
}

void fill_matrix2(double* vals, int* cols) {
    int row_count = 0;

    // Initialize vals and cols arrays
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < ROWSIZE; j++) {
            vals[i * ROWSIZE + j] = 0.0;
            cols[i * ROWSIZE + j] = -1;
        }
    }

    // Fill the main diagonal and inverse diagonal
    for (int i = 0; i < N; i++) {
        int count = 0;

        // Main diagonal
        vals[i * ROWSIZE + count] = 1.0;
        cols[i * ROWSIZE + count] = i;
        count++;

        // Inverse diagonal
        if (i != (N - 1 - i)) {
            vals[i * ROWSIZE + count] = 2.0;
            cols[i * ROWSIZE + count] = N - 1 - i;
            count++;
        }

        // Fill the remaining space in vals and cols for this row with 0.0 and -1
        while (count < ROWSIZE) {
            vals[i * ROWSIZE + count] = 0.0;
            cols[i * ROWSIZE + count] = -1;
            count++;
        }
    }
}

void print_matrix(int m, int n, int r, double* vals, int* cols) {
    double full_matrix[m][n];
    
    // Initialize the full matrix with zeros
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            full_matrix[i][j] = 0.0;
        }
    }

    // Populate the full matrix with values from vals and cols arrays
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < r; j++) {
            int col_index = cols[i * r + j];
            if (col_index != -1) {
                full_matrix[i][col_index] = vals[i * r + j];
            }
        }
    }

    // Print the full matrix
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%4.1f ", full_matrix[i][j]);
        }
        printf("\n");
    }
}


int main()
{
    int     vec_size = N*N;
    double  time_start, time_end, time_cpu, time_gpu;

    double* x     = (double*) malloc (vec_size*sizeof(double));
    double* y_cpu = (double*) malloc (vec_size*sizeof(double));
    double* y_gpu = (double*) malloc (vec_size*sizeof(double));

    double* Avals = (double*) malloc (ROWSIZE*vec_size*sizeof(double));
    int* Acols = (int*) malloc (ROWSIZE*vec_size*sizeof(int));



    // fill vector with sinusoidal for testing the code
    for(int i = 0; i < vec_size; i++)
    {
        x[i] = sin(i*0.01);
        y_cpu[i] = 0.0;
        y_gpu[i] = 0.0;
    }

    fill_matrix(Avals, Acols);
   


    time_start = omp_get_wtime();

    for(int i = 0; i < 100; i++)
        spmv_cpu(vec_size, ROWSIZE, Avals, Acols, x, y_cpu);
    
  

    time_end = omp_get_wtime();
    time_cpu = time_end - time_start;
    
    for(int i = 0; i<N; i++){
        printf("%d: %f\n", i, y_cpu[i]);
    }

    for (int i = 0; i < vec_size; i++) {
        y_gpu[i] = 0.0;
    }

    #pragma acc enter data copyin(Avals[:vec_size*ROWSIZE], Acols[:vec_size*ROWSIZE], x[:vec_size], y_gpu[:vec_size]) 
    time_start = omp_get_wtime();

    for(int i = 0; i < 100; i++)
        spmv_gpu(vec_size, ROWSIZE, Avals, Acols, x, y_gpu);


    time_end = omp_get_wtime();
    time_gpu = time_end - time_start;
    #pragma acc exit data copyout(y_gpu[:vec_size])

    for(int i = 0; i<N; i++){
        printf("%d: %f\n", i, y_gpu[i]);
    }


    // compare gpu and cpu results
    double norm2 = 0.0;
    for(int i = 0; i < vec_size; i++)
        norm2 += (y_cpu[i] - y_gpu[i])*(y_cpu[i] - y_gpu[i]);

    norm2 = sqrt(norm2);

    printf("spmv comparison cpu vs gpu error: %e, size %d\n",
           norm2, vec_size);

    printf("CPU Time: %lf\n", time_cpu);
    printf("GPU Time: %lf\n", time_gpu);

    // free allocated memory
    free(x);
    free(y_cpu);
    free(y_gpu);
    free(Avals);
    free(Acols);
}
