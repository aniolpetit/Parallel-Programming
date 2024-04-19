#!/bin/bash

#SBATCH --job-name=sort
#SBATCH -p ippd-cpu
#SBATCH --output=sort_%j.out
#SBATCH --error=sort_%j.err
#SBATCH --cpus-per-task=1
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --time=00:00:10

# Compile all versions
make all || exit 1  # Exit if make fails

# Execute with 2 threads
echo "Running sort program with 2 threads..."
./sort 10000000
echo

# Execute with 4 threads
echo "Running sort program with 4 threads..."
./sort 10000000
echo

# Execute with 8 threads
echo "Running sort program with 8 threads..."
./sort 10000000
echo

# Execute with 16 threads
echo "Running sort program with 16 threads..."
./sort 10000000
echo


