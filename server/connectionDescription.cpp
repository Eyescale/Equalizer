
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

#include "global.h"
#include <eq/base/log.h>
#include <eq/net/connection.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    hostname      = global->getConnectionSAttribute( SATTR_HOSTNAME );
    launchCommand = global->getConnectionSAttribute( SATTR_LAUNCH_COMMAND );

    launchTimeout = global->getConnectionIAttribute( IATTR_LAUNCH_TIMEOUT );
    type          = (eqNet::Connection::Type)global->
        getConnectionIAttribute( IATTR_TYPE );

    switch( type )
    {
        case eqNet::Connection::TYPE_TCPIP:
            TCPIP.port = global->getConnectionIAttribute( IATTR_TCPIP_PORT );
            break;
        default:
            break;
    }
}

std::ostream& eqs::operator << ( std::ostream& os, 
                                 const eqNet::ConnectionDescription* desc )
{
    os << disableFlush << "connection" << endl;
    os << "{" << endl << indent;

    const Global* global = Global::instance();

    if( desc->type != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_TYPE ))
        os << "type " 
           << ( desc->type == eqNet::Connection::TYPE_TCPIP ? "TCPIP" : 
                desc->type == eqNet::Connection::TYPE_PIPE ?  "PIPE" :
                "UNIPIPE" ) << endl;
    
    if( desc->TCPIP.port != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_TCPIP_PORT ))
        os << "TCPIP_port " << desc->TCPIP.port << endl;

    if( desc->launchTimeout != global->getConnectionIAttribute( 
            eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT ))
        os << "timeout " << desc->launchTimeout << endl;

    if( desc->hostname != global->getConnectionSAttribute( 
            eqs::ConnectionDescription::SATTR_HOSTNAME ))
        os << "hostname \"" << desc->hostname << "\"" << endl;

    if( desc->launchCommand != global->getConnectionSAttribute( 
            eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND ))
        os << "command \"" << desc->launchCommand << "\"" << endl;
    
    os << exdent << "}" << enableFlush << endl;

    return os;
}
