
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_CONFIG_PARAMS_H
#define EQFABRIC_CONFIG_PARAMS_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>
#include <co/types.h>


namespace eq
{
namespace fabric
{
namespace detail { class ConfigParams; }

/** Parameters for running a configuration. @sa Server::chooseConfig() */
class ConfigParams
{
public:
    /**
     * @warning Experimental - may not be supported in the future.
     * Flags influencing the configuration to be used or created.
     * @version 1.3.0
     */
    enum Flags
    {
        FLAG_NONE = LB_BIT_NONE, //!< Unset all flags
        FLAG_MULTIPROCESS = LB_BIT1, //!< Auto-config: one node per pipe
        FLAG_MULTIPROCESS_DB = LB_BIT2, //!< one node per DB decomposition
        FLAG_NETWORK_ETHERNET = LB_BIT3, //!< Auto-config: use ethernet only
        FLAG_NETWORK_INFINIBAND = LB_BIT4, //!< Auto-config: use IB only
        /** Auto-config: horizontal partition for load equalizer */
        FLAG_LOAD_EQ_HORIZONTAL = LB_BIT5,
        /** Auto-config: vertical partition for load equalizer */
        FLAG_LOAD_EQ_VERTICAL = LB_BIT6,
        /** Auto-config: 2D partition for load equalizer */
        FLAG_LOAD_EQ_2D = LB_BIT7,
        /** @internal */
        FLAG_LOAD_EQ_ALL = FLAG_LOAD_EQ_HORIZONTAL | FLAG_LOAD_EQ_VERTICAL |
                           FLAG_LOAD_EQ_2D,
        /** @internal */
        FLAG_NETWORK_ALL = FLAG_NETWORK_ETHERNET | FLAG_NETWORK_INFINIBAND
    };

    /** Construct new configuration parameters. @version 1.0 */
    EQFABRIC_API ConfigParams();

    /** Destruct this configuration parameters. @version 1.0 */
    EQFABRIC_API ~ConfigParams();

    /** @internal */
    EQFABRIC_API ConfigParams( const ConfigParams& rhs );

    /** @internal */
    EQFABRIC_API ConfigParams& operator = ( const ConfigParams& rhs );

    /** @name Data Access. */
    //@{
    /** Set the name of the configuration. @version 1.10 */
    EQFABRIC_API void setName( const std::string& name );

    /** @return the name of the configuration. @version 1.10 */
    EQFABRIC_API const std::string& getName() const;

    /**
     * Set the name of the render client executable.
     *
     * If no render client is provided, eq::Server::chooseConfig() uses the
     * program name retrieved from eq::Global::getProgramName(), i.e., the
     * filename part of argv[0].
     * @version 1.0
     */
    EQFABRIC_API void setRenderClient( const std::string& renderClient );

    /** @return the name of the render client executable. @version 1.0 */
    EQFABRIC_API const std::string& getRenderClient() const;

    /**
     * Set the command line arguments for the render client executable.
     *
     * If no arguments are provided, eq::Server::chooseConfig() uses
     * co::LocalNode::getCommandLine().
     */
    EQFABRIC_API void setRenderClientArgs( const Strings& args );

    /** @return the render client command line arguments */
    EQFABRIC_API const Strings& getRenderClientArgs() const;

    /** Set the prefixes of the environmental variables to pass on clients. */
    EQFABRIC_API void setRenderClientEnvPrefixes( const Strings& prefixes );

    /** @return prefixes of the environmental variables to pass on clients. */
    EQFABRIC_API const Strings& getRenderClientEnvPrefixes() const;

    /**
     * Set the directory from which to launch the render client.
     *
     * If no working directory is provided, eq::Server::chooseConfig() uses
     * eq::Global::getWorkDir(), i.e., the current working directory.
     * @version 1.0
     */
    EQFABRIC_API void setWorkDir( const std::string& workDir );

    /**
     * @return the directory from which to launch the render client.
     * @version 1.0
     */
    EQFABRIC_API const std::string& getWorkDir() const;

    /**
     * @warning Experimental - may not be supported in the future.
     * Set configuration flags. @version 1.3
     */
    EQFABRIC_API void setFlags( const uint32_t flags );

    /**
     * @warning Experimental - may not be supported in the future.
     * @return the configuration flags. @version 1.3
     */
    EQFABRIC_API uint32_t getFlags() const;

    /** @return read-access to Equalizer properties. @version 1.5.1 */
    EQFABRIC_API const Equalizer& getEqualizer() const;

    /** @return write-access to Equalizer properties. @version 1.5.1 */
    EQFABRIC_API Equalizer& getEqualizer();

    /**
     * Set a list of network prefixes in CIDR notation for autoconfig
     * network interface filtering.
     *
     * @version 1.5.1
     */
    EQFABRIC_API void setPrefixes( const Strings& prefixes );

    /** @return network prefixes in CIDR notation. @version 1.5.1 */
    EQFABRIC_API const Strings& getPrefixes() const;

    /** Set a regex filter matching to 'nodename:display.port' */
    EQFABRIC_API void setGPUFilter( const std::string& regex );

    /** @return the GPU regex filter. */
    EQFABRIC_API const std::string& getGPUFilter() const;
    //@}

    EQFABRIC_API void serialize( co::DataOStream& os ) const; //!< @internal
    EQFABRIC_API void deserialize( co::DataIStream& is ); //!< @internal

private:
    detail::ConfigParams* const _impl;
};

EQFABRIC_API co::DataOStream& operator << ( co::DataOStream& os,
                                            const ConfigParams& );

EQFABRIC_API co::DataIStream& operator >> ( co::DataIStream& is,
                                            ConfigParams& );
}
}

#endif // EQFABRIC_CONFIG_PARAMS_H
