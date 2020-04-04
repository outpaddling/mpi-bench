/* Linux */

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <mpi.h>
#include "mpi-bench.h"

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/cpuset.h>
#include <sys/types.h>
#include <sys/sysctl.h>

typedef cpuset_t cpu_set_t;

#define sched_getaffinity(pid, size, maskptr) \
    cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, \
    (pid) == 0 ? -1 : (pid), size, maskptr)

#endif

/***************************************************************************
 *  Description:
 *      This is a demo program that tests various performance measures of
 *      a cluster.
 *
 *      It also provides an example of a typical SPMD (Single Program
 *      Multiple Data) program, in which multiple processes run the
 *      same program, but may perform different tasks.
 *
 *  Usage:
 *      mpirun [flags] mpi_bench list-file
 *
 *  Returns:
 *      See sysexits.h
 *
 *  History: 
 *      Jan 2010    J Bacon
 ***************************************************************************/

int     main(int argc, char *argv[])

{
    int         proc_rank,
		proc_count,
		status,
		trials;
    char        *end;
    size_t      c, size;

    if ( argc != 2 )
	usage(argv);
    trials = strtol(argv[1], &end, 10);
    if ( *end != '\0' )
	usage(argv);

#if defined(__linux__)
    cpu_set_t   *mask;
    
    /* Linux code */
    mask = CPU_ALLOC(CPU_SETSIZE);
    size = CPU_ALLOC_SIZE(CPU_SETSIZE);
    CPU_ZERO_S(size, mask);
    if ( sched_getaffinity(0, size, mask) == -1 )
    {
	CPU_FREE(mask);
	perror("sched_getaffinity");
	return -1;
    }
    
    for ( c = 0; c < CPU_SETSIZE; c++ )
    {
	if ( CPU_ISSET_S(c, size, mask) )
	    printf("CPU %u is set\n", c);
    }
    
    CPU_FREE(mask);
#elif defined(__FreeBSD__)
    cpuset_t    mask;
    size = sizeof(mask);
    
    CPU_ZERO(&mask);
    
    if ( cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID,
			    -1, size, &mask) == -1 )
    {
	perror("sched_getaffinity");
	return -1;
    }

    for ( c = 0; c < CPU_SETSIZE; c++ )
    {
	if ( CPU_ISSET(c, &mask) )
	    printf("CPU %lu is set\n", c);
    }
#endif

    MPI_Init(&argc, &argv);

    /* Get rank of the calling process */
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    /* Get total number of processes in this run */
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

    /*
     *  Test broadcast throughput.  This is affected by network speed,
     *  topology, and MPI_Bcast() algorithm, which may be configurable
     *  for the MPI installation.  Must be run by all processes.
     */
    broadcast(proc_count);

    /* Test latency and throughput */
    if (proc_rank == RANK_ROOT)
	status = root_process(argc, argv, proc_count, trials);
    else
	status = non_root_process(trials);
    
    MPI_Finalize();

    return status;
}


/***************************************************************************
 *  Description:
 *      One process in SPMD programs is designated the "root" process
 *      and is usually responsible for coordinating the group.  This
 *      is not a rule, however, and in fact there are no rules.  SPMD
 *      processes can do whatever you want them to.
 *
 *  Arguments:
 *      argc, argv:     Command line arguments to the program
 *      proc_count:     Total number of processes in the group
 *
 *  Returns:
 *      See sysexits.h
 *
 *  History: 
 *      Jan 2010    J Bacon
 ***************************************************************************/

int     root_process(int argc, char *argv[], int proc_count, int trials)

{
    unsigned int    proc,
		    m,
		    trial;
    MPI_Status      mpi_status;
    static char     msg[MSG_SIZE];
    double          begin_time,
		    end_time,
		    elapsed_ms;
    char            hostname[128];
    
    /* Report existence of this process to user */
    gethostname(hostname, HOST_NAME_LEN);
    printf("Root host:  %s\n", hostname);
    printf("Trials:     %u\n", trials);
    printf("Proc count: %u\n", proc_count);
    
    /* Run several trials to drown out startup variability */
    for (trial = 1; trial <= trials; ++trial)
    {
	printf("\n==========\nTrial %u\n==========\n\n", trial);
	
	/* Send messages to all other processes */
	for (proc = 1; proc < proc_count; ++proc)
	{
	    MPI_Recv(hostname, HOST_NAME_LEN, MPI_CHAR, proc,
		    TAG_GENERIC, MPI_COMM_WORLD, &mpi_status);
	    
	    /* Send and receive 1 byte message to test latency */
	    DPRINTF("Process %u  Host %s\n", proc, hostname);
	
	    begin_time = MPI_Wtime();
	    // Causes program to hang after a couple iterations
	    for (m = 0; m < SMALL_MSG_COUNT; ++m)
	    {
		MPI_Send(msg, 1, MPI_CHAR, proc, TAG_GENERIC, MPI_COMM_WORLD);
		MPI_Recv(msg, 1, MPI_CHAR, proc, TAG_GENERIC, MPI_COMM_WORLD,
		    &mpi_status);
	    }
	    end_time = MPI_Wtime();
	    elapsed_ms = (end_time - begin_time) * MS_PER_SEC_DBL;
	    printf("Average 1-byte round-trip time over %u messages = %0.2fms\n",
		SMALL_MSG_COUNT, elapsed_ms / SMALL_MSG_COUNT);
    
	    /* Send and receive large message to test bandwidth */
	    begin_time = MPI_Wtime();
	    MPI_Send(msg, MSG_SIZE, MPI_CHAR,
		    proc, TAG_GENERIC, MPI_COMM_WORLD);
	    MPI_Recv(msg, 1, MPI_CHAR, proc, TAG_GENERIC,
		    MPI_COMM_WORLD, &mpi_status);
	    end_time = MPI_Wtime();
	    elapsed_ms = (end_time - begin_time) * MS_PER_SEC_DBL;
	    printf("%0.2fMB time = %0.2fms  Rate = %0.2f mbytes/s\n",
		    MSG_SIZE / MEG_DBL, elapsed_ms,
		    MSG_SIZE / (elapsed_ms * MS_PER_SEC_DBL));
	}
	
    }
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *      Code run by all but the root process.
 *
 *  Returns:
 *      See sysexits.h
 *
 *  History: 
 *      Jan 2010    J Bacon
 ***************************************************************************/

int     non_root_process(int trials)

{
    MPI_Status  mpi_status;
    static char msg[MSG_SIZE];
    int         trial,
		m;
    char        hostname[128];
    
    gethostname(hostname, HOST_NAME_LEN);
    
    for (trial = 1; trial <= trials; ++trial)
    {
	/* Report which host we're running on */
	MPI_Send(hostname, HOST_NAME_LEN, MPI_CHAR, RANK_ROOT, 
		TAG_GENERIC, MPI_COMM_WORLD);
    
	for (m = 0; m < SMALL_MSG_COUNT; ++m)
	{
	    /* Receive and acknowledge a 1 byte from root to test latency */
	    MPI_Recv(msg, 1, MPI_CHAR, RANK_ROOT, TAG_GENERIC, MPI_COMM_WORLD,
		    &mpi_status);
	    MPI_Send(msg, 1, MPI_CHAR, RANK_ROOT, TAG_GENERIC, MPI_COMM_WORLD);
	}
	
	/* Receive and acknowledge a large message from root to test bandwidth */
	MPI_Recv(msg, MSG_SIZE, MPI_CHAR, RANK_ROOT, TAG_GENERIC, MPI_COMM_WORLD,
		&mpi_status);
	MPI_Send(msg, 1, MPI_CHAR, RANK_ROOT, TAG_GENERIC, MPI_COMM_WORLD);
    }
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *      Benchmark broadcast operations.
 *
 *  Arguments:
 *      proc_count:     Total number of processes.
 *
 *  History: 
 *      Jan 2010    J Bacon
 ***************************************************************************/

void    broadcast(int proc_count)

{
    double          elapsed_ms,
		    begin_time,
		    end_time,
		    throughput;
    int             proc_rank,
		    sum;
    char            *msg = MALLOC(MSG_SIZE, char);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    
    begin_time = MPI_Wtime();
    
    /* Broadcast 1 byte from root to all processes */
    MPI_Bcast(msg, 1, MPI_CHAR, RANK_ROOT, MPI_COMM_WORLD);
    
    /* Collect broadcast message */
    MPI_Reduce(msg, &sum, 1, MPI_INT, MPI_SUM, RANK_ROOT, MPI_COMM_WORLD);
    
    /* Root process reports time when all messages have been collected */
    if ( proc_rank == RANK_ROOT )
    {
	end_time = MPI_Wtime();
	elapsed_ms = (end_time - begin_time) * MS_PER_SEC_DBL;
	printf("\n1 byte broadcast+reduce time = %0.2fms\n", elapsed_ms);
    }

    begin_time = MPI_Wtime();

    /* Broadcast large message from root to all processes */
    MPI_Bcast(msg, MSG_SIZE, MPI_CHAR, RANK_ROOT, MPI_COMM_WORLD);

    /* Collect broadcast message */
    MPI_Reduce(msg, &sum, 1, MPI_INT, MPI_SUM, RANK_ROOT, MPI_COMM_WORLD);

    /* Root process reports time when all messages have been collected */
    if ( proc_rank == RANK_ROOT )
    {
	end_time = MPI_Wtime();
	elapsed_ms = (end_time - begin_time) * MS_PER_SEC_DBL;

	/* Disregard data sent to self when computing throughput. */
	throughput = (double)MSG_SIZE * (proc_count - 1) /
	    (elapsed_ms * MS_PER_SEC_DBL);
	printf("\n%0.2fMB broadcast+reduce time = %0.2fms  Rate = %0.2f mbytes/s\n",
	    MSG_SIZE / MEG_DBL, elapsed_ms, throughput);
    }
    free(msg);
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2016-06-22  Jason Bacon Begin
 ***************************************************************************/

void    usage(char *argv[])

{
    fprintf(stderr, "Usage: %s trials\n", argv[0]);
    exit(EX_USAGE);
}

