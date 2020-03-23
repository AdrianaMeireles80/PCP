#!/bin/sh
#
#PBS -N tvp2
#PBS -l walltime = 1:00:00
#PBS -l nodes=1:r541:ppn=32
#PBS -q

THREADS="1 2 4 8 16 20 24 48"

module load gcc/4.9.3

for thread in $THREADS; do
	echo $thread
	export OMP_NUM_THREADS=$thread
	./a.out
done