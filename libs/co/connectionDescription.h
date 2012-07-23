
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_CONNECTIONDESCRIPTION_H
#define CO_CONNECTIONDESCRIPTION_H

#include <co/api.h>
#include <co/connectionType.h> // member enum
#include <co/types.h>

#include <lunchbox/referenced.h> // base class

namespace co
{
    /** Describes Connection parameters. */
    class ConnectionDescription : public lunchbox::Referenced
    {
    public:
        /** The network protocol for the connection. @version 1.0 */
        ConnectionType type;

        /** The bandwidth in kilobyte per second. @version 1.0 */
        int32_t bandwidth;

        /** The listening port (TCPIP, SDP, IB, MCIP, RDMA). @version 1.0 */
        uint16_t port;

        /** Construct a new, default description. @version 1.0 */
        ConnectionDescription() 
                : type( CONNECTIONTYPE_TCPIP )
                , bandwidth( 0 )
                , port( 0 )
                , _filename( "default" )
            {}

        /**
         * Construct a description from a string representation.
         *
         * The given data is consumed, that is, the data string should be empty
         * on return.
         *
         * @sa toString()
         * @version 1.0
         */
        ConnectionDescription( std::string& data );

        /** Serialize this description to a std::ostream. @version 1.0 */
        CO_API void serialize( std::ostream& os ) const;

        /** @return this description as a string. @version 1.0 */
        CO_API std::string toString() const;

        /** 
         * Read the connection description from a string.
         * 
         * The string is consumed as the description is parsed. Two different
         * formats are recognized, a human-readable and a machine-readable. The
         * human-readable version has the format
         * <code>hostname[:port][:type]</code> or
         * <code>filename:PIPE</code>. The <code>type</code> parameter can be
         * TCPIP, SDP, IB, MCIP, UDT or RSP. The machine-readable format
         * contains all connection description parameters and is not documented.
         *
         * @param data the string containing the connection description.
         * @return true if the information was read correctly, false if not.
         * @version 1.0
         */
        CO_API bool fromString( std::string& data );

        /** @name Data Access */
        //@{
        CO_API void setHostname( const std::string& hostname );
        CO_API const std::string& getHostname() const;

        CO_API void setInterface( const std::string& interfacename );
        CO_API const std::string& getInterface() const;

        CO_API void setFilename( const std::string& filename );
        CO_API const std::string& getFilename() const;

        CO_API bool isSameMulticastGroup( ConstConnectionDescriptionPtr rhs );

        /** @return true if the two descriptions have the same values. */
        CO_API bool operator == ( const ConnectionDescription& rhs ) const;

        /** @return true if the two descriptions have the different values. */
        bool operator != ( const ConnectionDescription& rhs ) const
            { return !( *this == rhs ); }
        //@}

    protected:
        virtual ~ConnectionDescription() {}

    private:
        /** The host name. */
        std::string _hostname;

        /** The host name of the interface (multicast). */
        std::string _interface;

        /** The name file using for a pipe. */
        std::string _filename;
    };

    CO_API std::ostream& operator << ( std::ostream&,
                                       const ConnectionDescription& );

    /** Serialize a vector of connection descriptions to a string. */
    CO_API std::string serialize( const ConnectionDescriptions& );

    /** 
     * Deserialize a vector or connection descriptions from a string.
     *
     * Consumes the data.
     * @param data The serialized connection descriptions.
     * @param descriptions return value, deserialized connection descriptions.
     * @return true on successful parsing, false otherwise.
     */
    CO_API bool deserialize( std::string& data,
                             ConnectionDescriptions& descriptions );
}

#endif // CO_CONNECTION_DESCRIPTION_H
