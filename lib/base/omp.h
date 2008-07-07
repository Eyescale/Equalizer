
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_OMP_H
#define EQBASE_OMP_H

#include <eq/base/base.h>     // for EQ_EXPORT
#include <eq/base/debug.h>    // for EQ_ERROR

#ifdef EQ_USE_OPENMP
#  include <omp.h>
#endif

namespace eq
{
namespace base
{
    /**
     * Base class abstracting omp
     */
    class EQ_EXPORT OMP 
    {
    public:
        /** Return the number of threads used in a parallel region. */
        static int getNThreads() { return _nThreads; }

    private:
        static int _nThreads;
    };
}

}
#endif //EQBASE_OMP_H
