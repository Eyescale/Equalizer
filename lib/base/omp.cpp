
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "omp.h"

namespace eqBase
{
static int _setupNThreads();

int OMP::_nThreads = _setupNThreads();

static int _setupNThreads()
{
#ifdef EQ_USE_OPENMP
    const char* nThreadsEnv = getenv( "OMP_NUM_THREADS" );
    int         nThreads    = 0;

    if( nThreadsEnv )
        nThreads = atoi( nThreadsEnv );

    if( nThreads < 1 )
        nThreads = omp_get_num_procs();

    EQASSERT( nThreads > 0 );
    omp_set_num_threads( nThreads );

    return nThreads;
#else
    return 1;
#endif
}
}
