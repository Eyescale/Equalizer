
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_OMP_H
#define COBASE_OMP_H

#include <co/base/api.h>

#ifdef CO_USE_OPENMP
#  include <omp.h>
#endif

namespace co
{
namespace base
{
    /** Base class for OpenMP functionality */
    class OMP 
    {
    public:
        /** 
         * @return the number of threads used in a parallel region.
         * @version 1.0
         */
        COBASE_API static unsigned getNThreads();
    };
}

}
#endif //COBASE_OMP_H
