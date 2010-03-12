
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_EYE_H
#define EQFABRIC_EYE_H

#include <eq/base/base.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /** Defines an eye pass. @version 1.0 */
    enum Eye
    {
        EYE_CYCLOP = 0,
        EYE_LEFT,
        EYE_RIGHT,
        EYE_ALL   // must be last
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Eye& eye );
}
}
#endif // EQFABRIC_EYE_H
