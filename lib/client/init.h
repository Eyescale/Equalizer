
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_INIT_H
#define EQ_INIT_H

#include <eq/base/base.h>

/** 
 * @namespace eq
 * @brief The Equalizer client library.
 *
 * This namespace implements the application-visible API to access the Equalizer
 * server.
 */
namespace eq
{
    class Config;
	class NodeFactory;

    /** 
     * Initialize the Equalizer client library.
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
	 * @param nodeFactory the factory for allocating Equalizer objects.
	 *
     * @return <code>true</code> if the library was successfully initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool init( const int argc, char** argv, NodeFactory* nodeFactory);
    
    /**
     * Deinitialize the Equalizer client library.
     *
     * @return <code>true</code> if the library was successfully de-initialised,
     *         <code>false</code> otherwise.
     */
    EQ_EXPORT bool exit();

    /**
     * Convenience function to retrieve a configuration.
     *
     * This function initializes a local client node, connects it to the server,
     * and retrieves a configuration. On any failure everything is correctly
     * deinitialized and 0 is returned.
     *
     * @return the pointer to a configuration, or 0 upon error.
     */
    EQ_EXPORT Config* getConfig( const int argc, char** argv );

   /** 
    * Convenience function to release a configuration.
    *
    * This function releases the configuration, disconnects the server,
    * and stops the local client node.
    */
    EQ_EXPORT void releaseConfig( Config* config );
}

#endif // EQNET_INIT_H

