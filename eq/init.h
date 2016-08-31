
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_INIT_H
#define EQ_INIT_H

#include <eq/api.h>
#include <eq/types.h>
#include <eq/version.h>    // used inline
#include <lunchbox/log.h>   // used inline

namespace eq
{
/** @internal */
EQ_API bool _init( const int argc, char** argv, NodeFactory* nodeFactory );

/**
 * Initialize the Equalizer client library.
 *
 * The following command line options are recognized by this function:
 * <ul>
 *   <li>--eq-logfile &lt;filename&gt; redirect all log output to the given
 *         file.</li>
 *   <li>--eq-server &lt;hostname&gt; to specify an explicit server
 *         address (cf. Global::setServer())</li>
 *   <li>--eq-config &lt;filename&gt; to specify the configuration file or hwsd
 *         session if an application-specific server is used (cf.
 *         Global::setConfigFile())</li>
 *   <li>--eq-render-client &lt;filename&gt; to specify an alternate name
 *         for the render client executable (default is argv[0]). Also sets
 *         the working directory to the director part of the filename.</li>
 * </ul>
 *
 * Please note that further command line parameters are recognized by
 * co::LocalNode::initLocal().
 *
 * exit() should be called independent of the return value of this function.
 *
 * @param argc the command line argument count.
 * @param argv the command line argument values.
 * @param nodeFactory the factory for allocating Equalizer objects.
 * @return true if the library was successfully initialized, false otherwise.
 */
inline bool init( const int argc, char** argv, NodeFactory* nodeFactory )
{
    if( EQ_VERSION_ABI == Version::getABI( ))
        return eq::_init( argc, argv, nodeFactory );
    LBWARN << "Equalizer shared library v" << Version::getABI()
           << " not binary compatible with application v" << EQ_VERSION_ABI
           << std::endl;
    return false;
}

/**
 * De-initialize the Equalizer client library.
 *
 * @return true if the library was successfully de-initialized, false otherwise.
 */
EQ_API bool exit();

/**
 * Convenience function to retrieve a configuration.
 *
 * This function initializes a local client node, connects it to the server,
 * and retrieves a configuration. On any failure everything is correctly
 * deinitialized and 0 is returned.
 *
 * @return the pointer to a configuration, or 0 upon error.
 */
EQ_API Config* getConfig( const int argc, char** argv );

/**
 * Convenience function to release a configuration.
 *
 * This function releases the configuration, disconnects the server,
 * and stops the local client node.
 */
EQ_API void releaseConfig( Config* config );
}

#endif // EQ_INIT_H
