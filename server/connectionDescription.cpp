
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
