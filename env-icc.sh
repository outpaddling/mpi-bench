
export SBATCH_CPU_BIND_VERBOSE=verbose

# Set up environment (PATH, LD_LIBRARY_PATH, etc.) for fluent
source /etc/bashrc
module purge
module load /sharedapps/modulefiles/icc/15.2
module load openmpi/1.10.2

