#!/bin/sh

#SBATCH --ntasks=2
#SBATCH --nodelist=compute-001

if [ $# != 1 ]; then
    printf "Usage: $0 freebsd|gcc|icc|pkgsrc|yum\n"
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
mpirun --report-bindings ./mpi-bench 400

# This works fine
# srun ./mpi-bench 400

# Ralph Castain diagnostic
#srun --cpu_bind=none ./mpi-bench 400

