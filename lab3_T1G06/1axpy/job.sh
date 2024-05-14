#!/bin/bash
#SBATCH --job-name=axpy
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err
#SBATCH --partition=ippd-gpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:01:00

module load NVHPC

make || exit 1
./axpy.x 10
./axpy.x 100
./axpy.x 1000
./axpy.x 10000
./axpy.x 100000
./axpy.x 1000000
./axpy.x 10000000
