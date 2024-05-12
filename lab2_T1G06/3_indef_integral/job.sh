#!/bin/bash
#SBATCH -J indef_integral
#SBATCH --output=out_%j.out
#SBATCH --error=out_%j.err
#SBATCH -p ippd-cpu
#SBATCH --time=00:02:00
#SBATCH --nodes=1
#SBATCH --ntasks=4         # Number of MPI tasks (processes)
#SBATCH --cpus-per-task=1  # Number of CPU cores per task

module load GCC/10.2.0
module load OpenMPI/4.1.2-GCC-10.2.0-with-slurm

make seq || exit 1  # Compile sequential version
make par || exit 1  # Compile parallel version

# Run sequential version
echo "Running sequential version..."
./indef_integral_seq 100000000 1000 

# Run parallel versions
echo "Running parallel version with 1 processes..."
mpirun -n 1 indef_integral_par 100000000 1000 

echo "Running parallel version with 2 processes..."
mpirun -n 2 indef_integral_par 100000000 1000 

echo "Running parallel version with 4 processes..."
mpirun -n 4 indef_integral_par 100000000 1000 


