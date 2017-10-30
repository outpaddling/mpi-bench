#!/bin/sh

#SBATCH --ntasks=24 --nodes=1
##SBATCH --nodelist=compute-001

if [ $# != 2 ]; then
    printf "Usage: $0 freebsd|gcc|icc|pkgsrc|yum procs\n"
    exit 1
fi

env=$1
. ./env-$env.sh

#printf "Environment:\n"
#printenv
#printf "End environment.\n"

# This results in MPI binding to its own cores instead of those assigned by
# SLURM.
#mpirun --mca btl self,sm ./mpi-bench
#mpirun -mca plm_base_verbose 15 ./mpi-bench
mpirun --report-bindings -np $2 ./mpi-bench 4000000

# This works fine
# srun ./mpi-bench 400

# Ralph Castain diagnostic
#srun --cpu_bind=none ./mpi-bench 400

