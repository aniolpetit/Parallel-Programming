#!/bin/bash

#SBATCH --job-name=pi
#SBATCH -p ippd-cpu
#SBATCH --output=sieve_%j.out
#SBATCH --error=sieve_%j.err
#SBATCH --cpus-per-task=1
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --time=00:00:10

make seq || exit 1  # Exit if make fails

./pi_seq 1000000