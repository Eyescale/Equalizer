
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_INIT_H
#define COBASE_INIT_H

#include <co/base/api.h>

namespace co
{
namespace base
{
    /** 
     * Initialize the Equalizer base classes.
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @return true if the library was successfully initialised, false otherwise
     * @version 1.0
     */
    COBASE_API bool init( const int argc, char** argv );

    /**
     * De-initialize the Equalizer base classes.
     *
     * @return <code>true</code> if the library was successfully de-initialised,
     *         <code>false</code> otherwise.
     * @version 1.0
     */
    COBASE_API bool exit();
}
}
#endif // COBASE_INIT_H

