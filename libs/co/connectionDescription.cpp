
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

#include "connectionDescription.h"

#include <sstream>

namespace co
{

#define SEPARATOR '#'

namespace
{
static ConnectionType _getConnectionType( const std::string& string )
{
    if( string == "TCPIP" )
        return CONNECTIONTYPE_TCPIP;
    if( string == "TCP" )
        return CONNECTIONTYPE_TCPIP;
    if( string == "SDP" )
        return CONNECTIONTYPE_SDP;
    if( string == "ANON_PIPE" )
        return CONNECTIONTYPE_PIPE;
    if( string == "PIPE" )
        return CONNECTIONTYPE_NAMEDPIPE;
    if( string == "IB" )
        return CONNECTIONTYPE_IB;
    if( string == "RSP" )
        return CONNECTIONTYPE_RSP;
    if( string == "RDMA" )
        return CONNECTIONTYPE_RDMA;
    if( string == "UDT" )
        return CONNECTIONTYPE_UDT;
    
    LBASSERTINFO( false, "Unknown type: " << string );
    return CONNECTIONTYPE_NONE;
}
}

ConnectionDescription::ConnectionDescription( std::string& data )
        : type( CONNECTIONTYPE_TCPIP )
        , bandwidth( 0 )
        , port( 0 )
        , filename( "default" )
{
    fromString( data );
    LBASSERTINFO( data.empty(), data );
}

std::string ConnectionDescription::toString() const
{
    std::ostringstream description;
    serialize( description );
    return description.str();
}

void ConnectionDescription::serialize( std::ostream& os ) const
{
    os << type << SEPARATOR << bandwidth << SEPARATOR << hostname  << SEPARATOR
       << interfacename << SEPARATOR << port << SEPARATOR << filename
       << SEPARATOR;
}

bool ConnectionDescription::fromString( std::string& data )
{
    {
        size_t nextPos = data.find( SEPARATOR );
        // assume hostname[:port][:type] or filename:PIPE format
        if( nextPos == std::string::npos )
        {
            type     = CONNECTIONTYPE_TCPIP;
            nextPos = data.find( ':' );
            if( nextPos == std::string::npos ) // assume hostname format
            {
                hostname = data;
                data.clear();
                return true;
            }

            hostname = data.substr( 0, nextPos );
            data      = data.substr( nextPos + 1 );

            while( nextPos != std::string::npos )
            {
                nextPos            = data.find( ':' );
                const std::string token = data.substr( 0, nextPos );
                data               = data.substr( nextPos + 1 );
                
                if( !token.empty() && isdigit( token[0] )) // port
                    port = atoi( token.c_str( ));
                else
                {
                    type = _getConnectionType( token );
                    if( type == CONNECTIONTYPE_NAMEDPIPE )
                    {
                        filename = hostname;
                        hostname.clear();
                    }
                    else if( type == CONNECTIONTYPE_NONE )
                        goto error;
                }
            }

            data.clear();
            return true;
        }

        // else assume SEPARATOR-delimited list
        const std::string typeStr = data.substr( 0, nextPos );
        data = data.substr( nextPos + 1 );

        type = _getConnectionType( typeStr );
        if( type == CONNECTIONTYPE_NONE )
            goto error;

        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;

        const std::string bandwidthStr = data.substr( 0, nextPos );
        data                      = data.substr( nextPos + 1 );
        bandwidth = atoi( bandwidthStr.c_str( ));
    
        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;

        hostname = data.substr( 0, nextPos );
        data      = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;

        interfacename = data.substr( 0, nextPos );
        data       = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;
        
        const std::string portStr = data.substr( 0, nextPos );
        data                 = data.substr( nextPos + 1 );
        port                 = atoi( portStr.c_str( ));

        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;

        filename = data.substr( 0, nextPos );
        data = data.substr( nextPos + 1 );
    }
    return true;

  error:
    LBWARN << "Could not parse connection description: " << data << std::endl;
    return false;
}

void ConnectionDescription::setHostname( const std::string& hostname_ )
{
    hostname = hostname_;
}

void ConnectionDescription::setInterface( const std::string& interface_ )
{
    interfacename = interface_;
}

void ConnectionDescription::setFilename( const std::string& filename_ )
{
    filename = filename_;
}

const std::string& ConnectionDescription::getFilename() const
{
    return filename;
}

const std::string& ConnectionDescription::getHostname() const
{
    return hostname;
}

const std::string& ConnectionDescription::getInterface() const
{
    return interfacename;
}

bool ConnectionDescription::isSameMulticastGroup(
    ConstConnectionDescriptionPtr rhs )
{
    return( type >= CONNECTIONTYPE_MULTICAST && type == rhs->type &&
            hostname == rhs->hostname && port == rhs->port );
}

bool ConnectionDescription::operator == ( const ConnectionDescription& rhs )
    const
{
    return type == rhs.type && bandwidth == rhs.bandwidth &&
           port == rhs.port && hostname == rhs.hostname &&
           interfacename == rhs.interfacename && filename == rhs.filename;
}

std::string serialize( const ConnectionDescriptions& descriptions )
{
    std::ostringstream data;
    data << descriptions.size() << CO_SEPARATOR;

    for( ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        ConnectionDescriptionPtr desc = *i;
        desc->serialize( data );
    }
    
    return data.str();
}

bool deserialize( std::string& data, ConnectionDescriptions& descriptions )
{
    if( !descriptions.empty( ))
        LBWARN << "Connection descriptions already hold data before deserialize"
               << std::endl;

    // num connection descriptions
    size_t nextPos = data.find( CO_SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        LBERROR << "Could not parse number of connection descriptions"
                << std::endl;
        return false;
    }

    const std::string sizeStr = data.substr( 0, nextPos );
    if( !isdigit( sizeStr[0] ))
    {
        LBERROR << "Could not parse number of connection descriptions"
                << std::endl;
        return false;
    }

    const size_t nDesc = atoi( sizeStr.c_str( ));
    data = data.substr( nextPos + 1 );

    // connection descriptions
    for( size_t i = 0; i < nDesc; ++i )
    {
        ConnectionDescriptionPtr desc = new ConnectionDescription;
        if( !desc->fromString( data ))
        {
            LBERROR << "Error during connection description parsing"
                    << std::endl;
            return false;
        }
        descriptions.push_back( desc );
    }

    return true;
}

std::ostream& operator << ( std::ostream& os, 
                            const ConnectionDescription& desc)
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "connection"
       << std::endl
       << "{" << std::endl << lunchbox::indent
       << "type          " << desc.type << std::endl
       << "hostname      \"" << desc.getHostname() << "\"" << std::endl;

    if( !desc.getInterface().empty( ))
        os << "interface     \"" << desc.getInterface() << "\"" << std::endl;

    if( desc.port != 0 )
        os << "port          " << desc.port << std::endl;

    if( !desc.getFilename().empty( ))
        os << "filename      \"" << desc.getFilename() << "\"" << std::endl;

    if( desc.bandwidth != 0 )
        os << "bandwidth     " << desc.bandwidth << std::endl;

    return os << lunchbox::exdent << "}" << lunchbox::enableHeader
              << lunchbox::enableFlush << std::endl;
}

}
