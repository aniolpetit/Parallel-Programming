#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "sort.h"
#include <limits.h>

int sort(int *array, int n) {
    int i, j, tmp;

    for (i=1;i<n;i++) {  
        tmp = array[i];  
        for(j=i-1;j >= 0 && array[j] > tmp;j--) {  
            array[j+1] = array[j];  
        }  
        array[j+1] = tmp;  
    }
  
}

int sort_openmp(int *array, int n){
    int i, j, tmp;
    int chunk_size = (n + _NUM_THREADS - 1) / _NUM_THREADS; // Calculate chunk size
    
    // Create temporary array for each thread
    int *tmp_array = (int *)malloc(sizeof(int) * n);
    
    // Parallel sorting phase
    #pragma omp parallel num_threads(_NUM_THREADS) shared(array, tmp_array, n, chunk_size) private(i, j, tmp)
    {
        int tid = omp_get_thread_num();
        int start = tid * chunk_size;
        int end = (tid + 1) * chunk_size;
        if (end > n) end = n; // Adjust end for last chunk
        
        // Each thread sorts its chunk of the array
        for (i = start; i < end; i++) {
            int tmp = array[i];
            // Find the correct position for the current element in the sorted part
            for (j = i - 1; j >= start && tmp_array[j] > tmp; j--) {
                tmp_array[j + 1] = tmp_array[j];
            }
            // Insert the current element at its correct position
            tmp_array[j + 1] = tmp;
        }
    }    
 
    // Save the first position of each thread
    int *head_i = (int *)malloc(sizeof(int) * _NUM_THREADS);
    int *is_finished = (int *)malloc(sizeof(int) * _NUM_THREADS);
    for (int i = 0; i < _NUM_THREADS; i++) {
        head_i[i] = i * chunk_size;
        is_finished[i] = 0; // Initialize to 0 to indicate segments are not yet finished
    }

    for (int i = 0; i < n; i++) {
        int min = INT_MAX;
        int control = 0;
        // Find the minimum element among the heads of the sorted segments
        for (int j = 0; j < _NUM_THREADS; j++) {
            if (!is_finished[j] && tmp_array[head_i[j]] < min) {
                min = tmp_array[head_i[j]];
                control = j;
            }
        }
        array[i] = min;
        // Move to the next element in the segment from which the minimum was selected
        head_i[control]++;
        // Check if the current segment is finished
        if (head_i[control] >= (control + 1) * chunk_size) {
            is_finished[control] = 1; // Mark the segment as finished
        }
    }
    free(head_i);
    free(is_finished);
}

void fill_array(int *array, int n) {
    int i;
    
    srand(time(NULL));
    for(i=0; i < n; i++) {
        array[i] = rand()%n;
    }
}
