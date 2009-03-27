
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
