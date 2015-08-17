#define BENCHMARK "OSU OpenSHMEM Barrier Latency Test"
/*
 * Copyright (C) 2002-2015 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University. 
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <shmem.h>
#include <stdlib.h>
#include "osu_common.h"
#include "osu_coll.h"

long pSyncBarrier1[_SHMEM_BARRIER_SYNC_SIZE];
long pSyncBarrier2[_SHMEM_BARRIER_SYNC_SIZE];
long pSyncRed1[_SHMEM_REDUCE_SYNC_SIZE];
long pSyncRed2[_SHMEM_REDUCE_SYNC_SIZE];

double pWrk1[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
double pWrk2[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];

int main(int argc, char *argv[])
{
    int i = 0, rank;
    int skip, numprocs;
    static double avg_time = 0.0, max_time = 0.0, min_time = 0.0;
    static double latency = 0.0;
    int64_t t_start = 0, t_stop = 0, timer=0;
    int full = 0, t;

    for ( t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1) pSyncRed1[t] = _SHMEM_SYNC_VALUE;
    for ( t = 0; t < _SHMEM_REDUCE_SYNC_SIZE; t += 1) pSyncRed2[t] = _SHMEM_SYNC_VALUE;
    for ( t = 0; t < _SHMEM_BARRIER_SYNC_SIZE; t += 1) pSyncBarrier1[t] = _SHMEM_SYNC_VALUE;
    for ( t = 0; t < _SHMEM_BARRIER_SYNC_SIZE; t += 1) pSyncBarrier2[t] = _SHMEM_SYNC_VALUE;

    start_pes(0);
    rank = _my_pe();
    numprocs = _num_pes();

    if (process_args(argc, argv, rank, NULL, &full)) {
        return EXIT_SUCCESS;
    }

    if(numprocs < 2) {
        if(rank == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }
        return EXIT_FAILURE;
    }

    print_header(rank, full);

    skip = SKIP_LARGE;
    iterations = iterations_large;
    timer=0;        

    for(i=0; i < iterations + skip ; i++) {
        t_start = TIME();
        if(i%2)
            shmem_barrier(0, 0, numprocs, pSyncBarrier1);
        else
            shmem_barrier(0, 0, numprocs, pSyncBarrier2);
        t_stop = TIME();

        if(i>=skip){
            timer+=t_stop-t_start;
        } 
    }
            
    shmem_barrier_all();

    latency = (1.0 * timer) / iterations;
    shmem_double_min_to_all(&min_time, &latency, 1, 0, 0, numprocs, pWrk1, pSyncRed1);
    shmem_double_max_to_all(&max_time, &latency, 1, 0, 0, numprocs, pWrk2, pSyncRed2);
    shmem_double_sum_to_all(&avg_time, &latency, 1, 0, 0, numprocs, pWrk1, pSyncRed1);

    avg_time = avg_time/numprocs;
    print_data(rank, full, 0, avg_time, min_time, max_time, iterations);

    return EXIT_SUCCESS;
}

/* vi: set sw=4 sts=4 tw=80: */
