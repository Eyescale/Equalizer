
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

#include "connectionDescription.h"

#include <sstream>

namespace co
{

#define SEPARATOR '#'
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CONNECTION_") + #attr )

namespace
{
static std::string _sAttributeStrings[ ConnectionDescription::SATTR_ALL ] =
{
    MAKE_ATTR_STRING( SATTR_HOSTNAME ),
    MAKE_ATTR_STRING( SATTR_PIPE_FILENAME ),
    MAKE_ATTR_STRING( SATTR_FILL1 ),
    MAKE_ATTR_STRING( SATTR_FILL2 )
};
static std::string _iAttributeStrings[ ConnectionDescription::IATTR_ALL ] =
{
    MAKE_ATTR_STRING( IATTR_TYPE ),
    MAKE_ATTR_STRING( IATTR_TCPIP_PORT ),
    MAKE_ATTR_STRING( IATTR_BANDWIDTH ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

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
    if( string == "UDP" )
        return CONNECTIONTYPE_UDP;
    if( string == "MCIP" )
        return CONNECTIONTYPE_MCIP;
    if( string == "PGM" )
        return CONNECTIONTYPE_PGM;
    if( string == "RSP" )
        return CONNECTIONTYPE_RSP;
    
    EQASSERTINFO( false, "Not implemented" );
    return CONNECTIONTYPE_NONE;
}
}

ConnectionDescription::ConnectionDescription( const char* data )
        : type( CONNECTIONTYPE_TCPIP )
        , bandwidth( 0 )
        , port( 0 )
        , _filename( "default" )
{
    std::string string( data );
    fromString( string );
    EQASSERTINFO( string.empty(), data << " -> " << string );
}

const std::string& ConnectionDescription::getSAttributeString(
    const SAttribute attr )
{
    return _sAttributeStrings[ attr ];
}

const std::string& ConnectionDescription::getIAttributeString( 
    const IAttribute attr )
{
    return _iAttributeStrings[ attr ];
}

std::string ConnectionDescription::toString() const
{
    std::ostringstream description;
    serialize( description );
    return description.str();
}

void ConnectionDescription::serialize( std::ostream& os ) const
{
    os << type << SEPARATOR << bandwidth << SEPARATOR << _hostname  << SEPARATOR
       << _interface << SEPARATOR << port << SEPARATOR << _filename
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
                _hostname = data;
                data.clear();
                return true;
            }

            _hostname = data.substr( 0, nextPos );
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
                        _filename = _hostname;
                        _hostname.clear();
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
        data                 = data.substr( nextPos + 1 );

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

        _hostname = data.substr( 0, nextPos );
        data      = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == std::string::npos )
            goto error;

        _interface = data.substr( 0, nextPos );
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

        _filename = data.substr( 0, nextPos );
        data = data.substr( nextPos + 1 );
    }
    return true;

  error:
    EQWARN << "Could not parse connection description: " << data << std::endl;
    return false;
}

void ConnectionDescription::setHostname( const std::string& hostname )
{
    _hostname = hostname;
}

void ConnectionDescription::setInterface( const std::string& interface )
{
    _interface = interface;
}

void ConnectionDescription::setFilename( const std::string& filename )
{
    _filename = filename;
}

const std::string& ConnectionDescription::getFilename() const
{
    return _filename;
}

const std::string& ConnectionDescription::getHostname() const
{
    return _hostname;
}

const std::string& ConnectionDescription::getInterface() const
{
    return _interface;
}

bool ConnectionDescription::isSameMulticastGroup( ConnectionDescriptionPtr rhs )
{
    return( type == rhs->type && _hostname == rhs->_hostname &&
            port == rhs->port );
}

std::ostream& operator << ( std::ostream& os, 
                            const ConnectionDescription& desc)
{
    os << co::base::disableFlush << co::base::disableHeader << "connection"
       << std::endl;
    os << "{" << std::endl << co::base::indent;

    os << "type          " 
       << ( desc.type == co::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
            desc.type == co::CONNECTIONTYPE_SDP   ? "SDP" : 
            desc.type == co::CONNECTIONTYPE_PIPE  ? "ANON_PIPE" :
            desc.type == co::CONNECTIONTYPE_NAMEDPIPE ? "PIPE" :
            desc.type == co::CONNECTIONTYPE_IB    ? "IB" :
            desc.type == co::CONNECTIONTYPE_MCIP  ? "MCIP" :
            desc.type == co::CONNECTIONTYPE_PGM   ? "PGM" :
            desc.type == co::CONNECTIONTYPE_RSP   ? "RSP" :
            "ERROR" ) << std::endl;
    
    os << "hostname      \"" << desc.getHostname() << "\"" << std::endl;

    if( !desc.getInterface().empty( ))
        os << "interface     \"" << desc.getInterface() << "\"" << std::endl;

    if( desc.port != 0 )
        os << "port          " << desc.port << std::endl;

    if( !desc.getFilename().empty( ))
        os << "filename      \"" << desc.getFilename() << "\"" << std::endl;

    if( desc.bandwidth != 0 )
        os << "bandwidth     " << desc.bandwidth << std::endl;

    os << co::base::exdent << "}" << co::base::enableHeader 
       << co::base::enableFlush
       << std::endl;
    return os;
}

bool ConnectionDescription::operator == ( const ConnectionDescription& rhs )
    const
{
    return type == rhs.type && bandwidth == rhs.bandwidth &&
           port == rhs.port && _hostname == rhs._hostname &&
           _interface == rhs._interface && _filename == rhs._filename;
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
        EQWARN << "Connection descriptions already hold data before deserialize"
               << std::endl;

    // num connection descriptions
    size_t nextPos = data.find( CO_SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse number of connection descriptions"
                << std::endl;
        return false;
    }

    const std::string sizeStr = data.substr( 0, nextPos );
    if( !isdigit( sizeStr[0] ))
    {
        EQERROR << "Could not parse number of connection descriptions"
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
            EQERROR << "Error during connection description parsing"
                    << std::endl;
            return false;
        }
        descriptions.push_back( desc );
    }

    return true;
}

}
