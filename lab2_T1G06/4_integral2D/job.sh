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

#Execute the program
mpirun -np 4 ./integral2D_par surface_10_10.dat 10 10



