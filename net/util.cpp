
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "util.h"

using namespace eqNet::priv;

void Util::parseAddress( const char* address, char hostname[MAXHOSTNAMELEN], 
                         ushort &port )
{
    ASSERT( address );

    const size_t len = strlen( address );
    for( size_t i=0; i<len && i<MAXHOSTNAMELEN; i++ )
    {
        if( address[i] == ':' )
        {
            memcpy( hostname, address, i-1 );
            hostname[i] = '\0';

            const char *portName = &address[i+1];
            port = (ushort)atoi( portName );

            return;
        }
    }

    strncpy( hostname, address, MAXHOSTNAMELEN );
    hostname[MAXHOSTNAMELEN-1] = '\0';
    port = DEFAULT_PORT;
}

