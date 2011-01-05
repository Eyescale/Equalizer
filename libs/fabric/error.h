
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_ERROR_H
#define EQFABRIC_ERROR_H

#include <co/base/error.h>

namespace eq
{
namespace fabric
{
    using co::base::ERROR_NONE;
    /** Defines errors produced by Equalizer classes. */
    enum Error
    {
        // ERROR_ = co::base::ERROR_CUSTOM,
        ERROR_CUSTOM = EQ_64KB,
    };

    /** Print the error in a human-readable format. @version 1.0 */
    inline std::ostream& operator << ( std::ostream& os, const Error& error)
        { os << co::base::Error( error ); return os; }
}
}
#endif // EQFABRIC_ERROR_H
