
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "omp.h"
#include "debug.h"

#include <cstdlib>

namespace co
{
namespace base
{
namespace
{
static unsigned _setupNThreads();
unsigned _nThreads = _setupNThreads();

static unsigned _setupNThreads()
{
#ifdef CO_USE_OPENMP
    const char* nThreadsEnv = getenv( "OMP_NUM_THREADS" );
    unsigned    nThreads    = 0;

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

unsigned OMP::getNThreads()
{
    return _nThreads;
}

}
}
