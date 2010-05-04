
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

#include "connectionDescription.h"

#include <sstream>

using namespace std;

namespace eq
{
namespace net
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

string ConnectionDescription::toString() const
{
    ostringstream description;
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
        if( nextPos == string::npos )
        {
            type     = CONNECTIONTYPE_TCPIP;
            nextPos = data.find( ':' );
            if( nextPos == string::npos ) // assume hostname format
            {
                _hostname = data;
                data.clear();
                return true;
            }

            _hostname = data.substr( 0, nextPos );
            data      = data.substr( nextPos + 1 );

            while( nextPos != string::npos )
            {
                nextPos            = data.find( ':' );
                const string token = data.substr( 0, nextPos );
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
        const string typeStr = data.substr( 0, nextPos );
        data                 = data.substr( nextPos + 1 );

        type = _getConnectionType( typeStr );
        if( type == CONNECTIONTYPE_NONE )
            goto error;

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        const string bandwidthStr = data.substr( 0, nextPos );
        data                      = data.substr( nextPos + 1 );
        bandwidth = atoi( bandwidthStr.c_str( ));
    
        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        _hostname = data.substr( 0, nextPos );
        data      = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        _interface = data.substr( 0, nextPos );
        data       = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;
        
        const string portStr = data.substr( 0, nextPos );
        data                 = data.substr( nextPos + 1 );
        port                 = atoi( portStr.c_str( ));

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        _filename = data.substr( 0, nextPos );
        data = data.substr( nextPos + 1 );;
    }
    return true;

  error:
    EQWARN << "Could not parse connection description: " << data << endl;
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

const string& ConnectionDescription::getHostname() const
{
    return _hostname;
}

const string& ConnectionDescription::getInterface() const
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
    os << base::disableFlush << base::disableHeader << "connection"
       << std::endl;
    os << "{" << std::endl << base::indent;

    os << "type          " 
       << ( desc.type == net::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
            desc.type == net::CONNECTIONTYPE_SDP   ? "SDP" : 
            desc.type == net::CONNECTIONTYPE_PIPE  ? "ANON_PIPE" :
            desc.type == net::CONNECTIONTYPE_NAMEDPIPE ? "PIPE" :
            desc.type == net::CONNECTIONTYPE_IB    ? "IB" :
            desc.type == net::CONNECTIONTYPE_MCIP  ? "MCIP" :
            desc.type == net::CONNECTIONTYPE_PGM   ? "PGM" :
            desc.type == net::CONNECTIONTYPE_RSP   ? "RSP" :
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

    os << base::exdent << "}" << base::enableHeader << base::enableFlush
       << std::endl;
    return os;
}

}
}
