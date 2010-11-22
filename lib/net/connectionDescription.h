
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_CONNECTIONDESCRIPTION_H
#define EQNET_CONNECTIONDESCRIPTION_H

#include <eq/net/connectionType.h> // member enum
#include <eq/net/base.h>
#include <eq/net/types.h>

#include <eq/base/base.h>
#include <eq/base/referenced.h>

namespace eq
{
namespace net
{
    /**
     * Describes Connection parameters.
     *
     * @sa Node
     */
    class ConnectionDescription : public base::Referenced
    {
    public:
        ConnectionDescription() 
                : type( CONNECTIONTYPE_TCPIP )
                , bandwidth( 0 )
                , port( 0 )
                , _filename( "default" )
            {}

        /** The network protocol for the connection. */
        ConnectionType type;

        /** The bandwidth in kilobyte per second for this connection. */
        int32_t bandwidth;

        /** The listening port (TCPIP, SDP, IB, MCIP). */
        uint16_t port;

        /** @return this description as a string. */
        EQ_NET_DECL std::string toString() const;
        EQ_NET_DECL void serialize( std::ostream& os ) const;

        /** 
         * Reads the connection description from a string.
         * 
         * The string is consumed as the description is parsed. Two different
         * formats are recognized, a human-readable and a machine-readable. The
         * human-readable version has the format
         * <code>hostname[:port][:type]</code> or
         * <code>filename:PIPE</code>. The <code>type</code> parameter can be
         * TCPIP, SDP, IB, MCIP, PGM or RSP. The machine-readable format
         * contains all connection description parameters and is not documented.
         *
         * @param data the string containing the connection description.
         * @return <code>true</code> if the information was read correctly, 
         *         <code>false</code> if not.
         */
        EQ_NET_DECL bool fromString( std::string& data );

        /** @name Data Access
         *
         * std::strings are not public because of DLL allocation issues.
         */
        //@{
        EQ_NET_DECL void setHostname( const std::string& hostname );
        EQ_NET_DECL const std::string& getHostname() const;

        EQ_NET_DECL void setInterface( const std::string& interfacename );
        EQ_NET_DECL const std::string& getInterface() const;

        EQ_NET_DECL void setFilename( const std::string& filename );
        EQ_NET_DECL const std::string& getFilename() const;

        EQ_NET_DECL bool isSameMulticastGroup( ConnectionDescriptionPtr rhs );
        //@}

        /** @name Attributes */
        //@{
        // Note: also update string array init in connectionDescription.cpp
        /** String attributes */
        enum SAttribute
        {
            SATTR_HOSTNAME,
            SATTR_FILENAME,
            SATTR_FILL1,
            SATTR_FILL2,
            SATTR_ALL
        };

        /** Integer attributes */
        enum IAttribute
        {
            IATTR_TYPE,
            IATTR_PORT,
            IATTR_BANDWIDTH,
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };
        //@}

        EQ_NET_DECL static const std::string&
        getSAttributeString( const SAttribute attr );
        EQ_NET_DECL static const std::string&
        getIAttributeString( const IAttribute attr );

    protected:
        EQ_NET_DECL virtual ~ConnectionDescription() {}

    private:
        /** The host name. */
        std::string _hostname;

        /** The host name of the interface (multicast). */
        std::string _interface;

        /** The name file using for a pipe. */
        std::string _filename;
    };

    EQ_NET_DECL std::ostream& operator << ( std::ostream&,
                                          const ConnectionDescription& );

    /** Serialize a vector of connection descriptions to a string. */
    EQ_NET_DECL std::string serialize( const ConnectionDescriptions& );

    /** 
     * Deserialize a vector or connection descriptions from a string.
     *
     * Consumes the data.
     * @param data The serialized connection descriptions.
     * @param descriptions return value, deserialized connection descriptions.
     * @return true on successful parsing, false otherwise.
     */
    EQ_NET_DECL bool deserialize( std::string& data,
                                  ConnectionDescriptions& descriptions );
}
}

#endif // EQNET_CONNECTION_DESCRIPTION_H
