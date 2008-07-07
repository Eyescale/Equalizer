
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

#include "global.h"
#include <eq/base/log.h>
#include <eq/net/connection.h>

using namespace eq::base;
using namespace std;

namespace eqs
{
#define MAKE_ATTR_STRING( attr ) ( string("EQ_CONNECTION_") + #attr )

std::string ConnectionDescription::_sAttributeStrings[SATTR_ALL] = {
    MAKE_ATTR_STRING( SATTR_HOSTNAME ),
    MAKE_ATTR_STRING( SATTR_LAUNCH_COMMAND )
};
std::string ConnectionDescription::_cAttributeStrings[CATTR_ALL] = {
    MAKE_ATTR_STRING( CATTR_LAUNCH_COMMAND_QUOTE ),
};
std::string ConnectionDescription::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_TYPE ),
    MAKE_ATTR_STRING( IATTR_TCPIP_PORT ),
    MAKE_ATTR_STRING( IATTR_LAUNCH_TIMEOUT )
};

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    setHostname( global->getConnectionSAttribute( SATTR_HOSTNAME ));
    setLaunchCommand( global->getConnectionSAttribute( SATTR_LAUNCH_COMMAND ));

    launchCommandQuote = global->getConnectionCAttribute( 
                             CATTR_LAUNCH_COMMAND_QUOTE );

    launchTimeout  = global->getConnectionIAttribute( IATTR_LAUNCH_TIMEOUT );
    type           = (eq::net::ConnectionType)global->
        getConnectionIAttribute( IATTR_TYPE );

    switch( type )
    {
        case eq::net::CONNECTIONTYPE_TCPIP:
        case eq::net::CONNECTIONTYPE_SDP:
            TCPIP.port = global->getConnectionIAttribute( IATTR_TCPIP_PORT );
            break;
        default:
            break;
    }
}

std::ostream& operator << ( std::ostream& os, 
                            const eq::net::ConnectionDescription* desc )
{
    os << disableFlush << "connection" << endl;
    os << "{" << endl << indent;

    const Global* global = Global::instance();

    if( desc->type != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_TYPE ))
        os << "type          " 
           << ( desc->type == eq::net::CONNECTIONTYPE_TCPIP ? "TCPIP" : 
                desc->type == eq::net::CONNECTIONTYPE_SDP   ? "SDP" : 
                desc->type == eq::net::CONNECTIONTYPE_PIPE  ? "PIPE" :
                "ERROR" ) << endl;
    
    if( desc->TCPIP.port != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_TCPIP_PORT ))
        os << "TCPIP_port    " << desc->TCPIP.port << endl;

    if( desc->launchTimeout != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT ))
        os << "timeout       " << desc->launchTimeout << endl;

    if( desc->getHostname() != global->getConnectionSAttribute( 
            eqs::ConnectionDescription::SATTR_HOSTNAME ))
        os << "hostname      \"" << desc->getHostname() << "\"" << endl;

    if( desc->getLaunchCommand() != global->getConnectionSAttribute( 
            eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND ))
        os << "command       \"" << desc->getLaunchCommand() << "\"" << endl;
    
    if( desc->launchCommandQuote != global->getConnectionCAttribute( 
            eqs::ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE ))
        os << "command_quote '" << desc->launchCommandQuote << "'" << endl;
    
    os << exdent << "}" << enableFlush << endl;

    return os;
}
}
