
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "connectionDescription.h"

#include "global.h"
#include <eq/base/log.h>
#include <eq/net/connection.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
#define MAKE_ATTR_STRING( attr ) ( string("EQ_CONNECTION_") + #attr )

std::string ConnectionDescription::_sAttributeStrings[SATTR_ALL] = {
    MAKE_ATTR_STRING( SATTR_HOSTNAME ),
    MAKE_ATTR_STRING( SATTR_LAUNCH_COMMAND ),
    MAKE_ATTR_STRING( SATTR_FILL1 ),
    MAKE_ATTR_STRING( SATTR_FILL2 )
};
std::string ConnectionDescription::_cAttributeStrings[CATTR_ALL] = {
    MAKE_ATTR_STRING( CATTR_LAUNCH_COMMAND_QUOTE ),
    MAKE_ATTR_STRING( CATTR_FILL1 ),
    MAKE_ATTR_STRING( CATTR_FILL2 )
};
std::string ConnectionDescription::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_TYPE ),
    MAKE_ATTR_STRING( IATTR_TCPIP_PORT ),
    MAKE_ATTR_STRING( IATTR_LAUNCH_TIMEOUT ),
    MAKE_ATTR_STRING( IATTR_BANDWIDTH ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    setHostname( global->getConnectionSAttribute( SATTR_HOSTNAME ));
    setLaunchCommand( global->getConnectionSAttribute( SATTR_LAUNCH_COMMAND ));

    launchCommandQuote = global->getConnectionCAttribute( 
                             CATTR_LAUNCH_COMMAND_QUOTE );

    launchTimeout  = global->getConnectionIAttribute( IATTR_LAUNCH_TIMEOUT );
    type           = static_cast< net::ConnectionType >(
                         global->getConnectionIAttribute( IATTR_TYPE ));

    bandwidth = global->getConnectionIAttribute( IATTR_BANDWIDTH );

    switch( type )
    {
        case net::CONNECTIONTYPE_TCPIP:
        case net::CONNECTIONTYPE_SDP:
            TCPIP.port = global->getConnectionIAttribute( IATTR_TCPIP_PORT );
            break;
        default:
            break;
    }
}

std::ostream& operator << ( std::ostream& os, 
                            const ConnectionDescription* desc )
{
    os << disableFlush << "connection" << endl;
    os << "{" << endl << indent;

    const Global* global = Global::instance();

    if( desc->type != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_TYPE ))
        os << "type          " 
           << ( desc->type == net::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
                desc->type == net::CONNECTIONTYPE_SDP   ? "SDP" : 
                desc->type == net::CONNECTIONTYPE_PIPE  ? "PIPE" :
                "ERROR" ) << endl;
    
    if( desc->TCPIP.port != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_TCPIP_PORT ))
        os << "TCPIP_port    " << desc->TCPIP.port << endl;

    if( desc->bandwidth != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_BANDWIDTH ))
        os << "bandwidth     " << desc->bandwidth << endl;

    if( desc->launchTimeout != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_LAUNCH_TIMEOUT ))
        os << "timeout       " << desc->launchTimeout << endl;

    if( desc->getHostname() != global->getConnectionSAttribute( 
            ConnectionDescription::SATTR_HOSTNAME ))
        os << "hostname      \"" << desc->getHostname() << "\"" << endl;

    if( desc->getLaunchCommand() != global->getConnectionSAttribute( 
            ConnectionDescription::SATTR_LAUNCH_COMMAND ))
        os << "command       \"" << desc->getLaunchCommand() << "\"" << endl;
    
    if( desc->launchCommandQuote != global->getConnectionCAttribute( 
            ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE ))
        os << "command_quote '" << desc->launchCommandQuote << "'" << endl;
    
    os << exdent << "}" << enableFlush << endl;

    return os;
}

}
}
