
# The module command is defined in the shell startup scripts.
# Bourne-family shell scripts don't source startup scripts, so we have to
# do it manually.
source /etc/bashrc
module purge
module load /sharedapps/modulefiles/Base-GCC/4.4/openmpi/1.10.2

