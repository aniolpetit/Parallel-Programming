#!/bin/bash
#SBATCH -J integral_2D
#SBATCH --output=out_%j.out
#SBATCH --error=out_%j.err
#SBATCH -p ippd-cpu
#SBATCH --nodes=1  
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1

# Load necessary modules
module load GCC/10.2.0
module load OpenMPI/4.1.2-GCC-10.2.0

# Compile the program
mpicc -o integral2D_par integral2D_par.c -lm -fopenmp

# Run the program
export OMP_NUM_THREADS=1  # Set the number of threads per MPI process
mpirun -np 4 ./integral2D_par surface_1000_1000.dat 1000 1000
