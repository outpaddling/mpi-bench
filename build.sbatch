#!/bin/sh

#SBATCH --ntasks=1

if [ $# != 1 ]; then
    printf "Usage: $0 freebsd|gcc|icc|pkgsrc|yum\n"
    exit 1
fi

env=$1
. ./env-$env.sh

echo $PATH
which mpicc
make clean
make

