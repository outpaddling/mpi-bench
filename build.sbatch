#!/bin/sh

#SBATCH --ntasks=1

if [ $# != 1 ]; then
    printf "Usage: $0 freebsd|gcc|icc|pkgsrc|yum\n"
    exit 1
fi

env=$1
. ./env-$env.sh

printf "$PATH\n"
which mpicc
make clean
make CC=mpicc LD=mpicc

