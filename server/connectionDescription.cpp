
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
    MAKE_ATTR_STRING( SATTR_PIPE_FILENAME ),
    MAKE_ATTR_STRING( SATTR_FILL1 ),
    MAKE_ATTR_STRING( SATTR_FILL2 )
};
std::string ConnectionDescription::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_TYPE ),
    MAKE_ATTR_STRING( IATTR_TCPIP_PORT ),
    MAKE_ATTR_STRING( IATTR_BANDWIDTH ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    setHostname( global->getConnectionSAttribute( SATTR_HOSTNAME ));
    type = static_cast< net::ConnectionType >(
        global->getConnectionIAttribute( IATTR_TYPE ));

    bandwidth = global->getConnectionIAttribute( IATTR_BANDWIDTH );

    port = global->getConnectionIAttribute( IATTR_PORT );
    setFilename( global->getConnectionSAttribute( SATTR_FILENAME));
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
                desc->type == net::CONNECTIONTYPE_PIPE  ? "ANON_PIPE" :
                desc->type == net::CONNECTIONTYPE_NAMEDPIPE  ? "PIPE" :
                desc->type == net::CONNECTIONTYPE_IB  ? "IB" :
                desc->type == net::CONNECTIONTYPE_MCIP  ? "MCIP" :
                desc->type == net::CONNECTIONTYPE_MCIP_PGM  ? "MCIP" :
                "ERROR" ) << endl;
    
    if( desc->getHostname() != global->getConnectionSAttribute( 
            ConnectionDescription::SATTR_HOSTNAME ))
        os << "hostname      \"" << desc->getHostname() << "\"" << endl;

    if( !desc->getInterface().empty( ))
        os << "interface     \"" << desc->getInterface() << "\"" << endl;

    if( desc->port != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_PORT ))
        os << "port          " << desc->port << endl;

    if( desc->getFilename() != global->getConnectionSAttribute( 
            ConnectionDescription::SATTR_FILENAME ))
        os << "filename \"" << desc->getFilename() << "\"" << endl;

    if( desc->bandwidth != global->getConnectionIAttribute( 
            ConnectionDescription::IATTR_BANDWIDTH ))
        os << "bandwidth     " << desc->bandwidth << endl;

    os << exdent << "}" << enableFlush << endl;

    return os;
}

}
}
