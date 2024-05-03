#!/bin/bash
#SBATCH -J def_integral
#SBATCH --output=out_%j.out
#SBATCH --error=out_%j.err
#SBATCH -p ippd-cpu
#SBATCH --time=00:02:00
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1

module load GCC/10.2.0
module load OpenMPI/4.1.2-GCC-10.2.0-with-slurm



make seq || exit 1  # Compile sequential version
make par || exit 1  # Compile parallel version

# Run sequential version
echo "Running sequential version..."
./def_integral_seq 1000000000 1000 
# Run parallel version
echo "Running parallel version..."
mpirun -n 4 def_integral_par 1000000000 1000 


