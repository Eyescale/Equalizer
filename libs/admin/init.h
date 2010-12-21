
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

#ifndef EQADMIN_INIT_H
#define EQADMIN_INIT_H

#include <eq/admin/api.h>

namespace eq
{
/**
 * @brief The Equalizer administrative library.
 *
 * This namespace implements the administrative interface to the Equalizer
 * server. It can be used in coexistence with the client library in the same
 * application.
 *
 * The administrative interface allows to query and modify the settings of
 * Equalizer servers, including the modification of a running configuration. An
 * administrative application connects to a Server, which will create a proxy of
 * the server configuration. This proxy can be sync'ed, modified and commit'ed
 * to track changes, change a configuration and activate the modifications,
 * respectively.
 */
namespace admin
{
    class NodeFactory;

    /**
     * Initialize the Equalizer administrative library.
     *
     * This function also initializes the network layer using co::init(),
     * if eq::init() was not called beforehand by the calling process. It has to
     * be called before any other access to classes or functions in this
     * namespace. It can be called before or after eq::init().
     *
     * @param argc the command line argument count.
     * @param argv the command line argument values.
     * @return <code>true</code> if the library was successfully initialized,
     *         <code>false</code> otherwise.
     */
    EQADMIN_EXPORT bool init( const int argc, char** argv );
    
    /**
     * De-initialize the Equalizer administrative library.
     *
     * This function also de-initializes the network layer, if the Equalizer
     * client library is not initialized.
     *
     * @return <code>true</code> if the library was successfully de-initialized,
     *         <code>false</code> otherwise.
     */
    EQADMIN_EXPORT bool exit();
}
}

#endif // EQADMIN_INIT_H

