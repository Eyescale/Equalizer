
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_INIT_H
#define EQSERVER_INIT_H

#include <co/base/os.h>

/** @file server/init.h */
namespace eq
{
/** 
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework. The API is not stable and certain assumptions are not documented,
 * use it at your own risk!
 */
namespace server
{
    /**
     * Initialize the Equalizer server namespace.
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     *
     * @return <code>true</code> if the library was successfully initialized,
     *         <code>false</code> otherwise.
     */
    EQSERVER_EXPORT bool init( const int argc, char** argv );
    
    /**
     * De-initialize the Equalizer server namespace.
     *
     * @return <code>true</code> if the library was successfully de-initialized,
     *         <code>false</code> otherwise.
     */
    EQSERVER_EXPORT bool exit();
}
}
#endif // EQSERVER_INIT_H
