#!/bin/sh
#
#PBS -N tp3
#PBS -l walltime=1:00:00
#PBS -l nodes=1:r662:ppn=48


module load gcc/4.9.0
gcc -fopenmp -std=c99 tp3.c


THREADS="2 4 8 16 20 24 48"

for thread in $THREADS; do
        echo $thread

            for i in {1..5}
            do

            echo "Teste numero $i :"
            export OMP_NUM_THREADS=$thread
            export OMP_PROC_BIND=master
            export OMP_PLACES=sockets           
               
            ./a.out
            done
done  
