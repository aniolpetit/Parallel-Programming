#!/bin/bash

#SBATCH --job-name=primes
#SBATCH -p ippd-cpu
#SBATCH --output=primes_%j.out
#SBATCH --error=primes_%j.err
#SBATCH --cpus-per-task=1
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --time=00:00:10

# Compile all versions
make all || exit 1  # Exit if make fails

./primes

