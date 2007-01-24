
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "util.h"

#include <eq/base/base.h>
#include <eq/base/debug.h>

#include <string.h>

using namespace eqNet::priv;

void Util::parseAddress( const char* address, char hostname[MAXHOSTNAMELEN], 
                         uint16_t &port )
{
    EQASSERT( address );

    const size_t len = strlen( address );
    for( size_t i=0; i<len && i<MAXHOSTNAMELEN; i++ )
    {
        if( address[i] == ':' )
        {
            memcpy( hostname, address, i );
            hostname[i] = '\0';

            const char *portName = &address[i+1];
            port = static_cast<uint16_t>( atoi( portName ));

            return;
        }
    }

    strncpy( hostname, address, MAXHOSTNAMELEN );
    hostname[MAXHOSTNAMELEN-1] = '\0';
    port = EQ_DEFAULT_PORT;
}

