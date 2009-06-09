
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_INIT_H
#define EQNET_INIT_H

#include <eq/base/init.h>

namespace eq
{
/** 
 * @namespace eq::net
 * @brief Equalizer network layer.
 *
 * The Equalizer network layer provides basic functionality for network
 * communication, such as Connection and ConnectionSet, as well as higher-level
 * functionality such as Node, Session and Object.
 */
namespace net
{
    class Node;

    /** 
     * Initialize the Equalizer network classes.
     * 
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @return <code>true</code> if the library was successfully initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool init( const int argc, char** argv );

    /**
     * De-initialize the Equalizer network classes.
     *
     * @return <code>true</code> if the library was successfully de-initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool exit();
}
}

#endif // EQNET_INIT_H

