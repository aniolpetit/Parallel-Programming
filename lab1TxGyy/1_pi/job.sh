#!/bin/bash

#SBATCH --job-name=pi
#SBATCH -p ippd-cpu
#SBATCH --output=pi_%j.out
#SBATCH --error=pi_%j.err
#SBATCH --cpus-per-task=1
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --time=00:00:10

# Compile all versions
make all || exit 1  # Exit if make fails

# Execute sequential version
echo "Running sequential version..."
./pi_seq 1048576
echo

# Execute parallel version
echo "Running parallel version..."
./pi_par 1048576
echo

# Execute task-based parallel version
echo "Running task-based parallel version..."
./pi_task 1048576 1024
echo