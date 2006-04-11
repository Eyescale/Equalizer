
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

using namespace eqs;

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    hostname      = global->getConnectionSAttribute( SATTR_HOSTNAME );
    launchCommand = global->getConnectionSAttribute( SATTR_LAUNCH_COMMAND );

    launchTimeout = global->getConnectionIAttribute( IATTR_LAUNCH_TIMEOUT );
    type          = global->getConnectionIAttribute( IATTR_TYPE );

    switch( type )
    {
        case Connection::TYPE_TCPIP:
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

    if( desc->type != global->getConnectionIAttribute( IATTR_TYPE ))
        os << "type " << ( desc->type == Connection::TYPE_TCPIP ? "TCPIP" : 
                           desc->type == Connection::TYPE_PIPE ?  "PIPE" :
                           "UNIPIPE" ) << endl;
    
    if( desc->port != global->getConnectionIAttribute( IATTR_TCPIP_PORT ))
        os << "TCPIP_port " << desc->port << endl;

    if( desc->launchTimeout != 
        global->getConnectionIAttribute( IATTR_LAUNCH_TIMEOUT ))
        os << "launchTimeout " << desc->launchTimeout << endl;

    if( desc->hostname != global->getConnectionSAttribute( SATTR_HOSTNAME ))
        os << "hostname \"" << desc->hostname << "\"" << endl;

    if( desc->launchCommand != 
        global->getConnectionSAttribute( SATTR_LAUNCH_COMMAND ))
        os << "launchCommand \"" << desc->launchCommand << "\"" << endl;
    
    os << exdent << "}" << enableFlush << endl;

    return os;
}
