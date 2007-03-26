
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

#include <sstream>

using namespace eqNet;
using namespace std;

#define SEPARATOR '#'

string ConnectionDescription::toString()
{
    ostringstream description;

    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
            description << "TCPIP";
            break;

        case CONNECTIONTYPE_PIPE:
            description << "PIPE";
            break;
    }        

    description << SEPARATOR << bandwidthKBS << SEPARATOR << launchCommand 
                << SEPARATOR << launchTimeout << SEPARATOR << hostname ;
    
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
            description << SEPARATOR << TCPIP.port;
            break;

        default:
            break;
    }
    return description.str();
}

bool ConnectionDescription::fromString( const string& data )
{
    {
        size_t colonPos = data.find( SEPARATOR );
        if( colonPos == string::npos ) // assume hostname:port format
        {
            type     = CONNECTIONTYPE_TCPIP;
            colonPos = data.find( ':' );
            if( colonPos == string::npos ) // assume hostname
            {
                hostname = data;
                return true;
            }

            hostname   = data.substr( 0, colonPos );
            ++colonPos;
            const string port = data.substr( colonPos, data.length()-colonPos );
            TCPIP.port = atoi( port.c_str( ));
            return true;
        }

        const string type = data.substr( 0, colonPos );

        if( type == "TCPIP" )
            this->type = CONNECTIONTYPE_TCPIP;
        else if( type == "PIPE" )
            this->type = CONNECTIONTYPE_PIPE;
        else
            goto error;

        size_t nextPos = colonPos+1;
        colonPos       = data.find( SEPARATOR, nextPos );
        if( colonPos == string::npos )
            goto error;
        const string bandwidth = data.substr( nextPos, colonPos-nextPos );
        bandwidthKBS = atoi( bandwidth.c_str( ));
    
        nextPos  = colonPos+1;
        colonPos = data.find( SEPARATOR, nextPos );
        if( colonPos == string::npos )
            goto error;
        launchCommand = data.substr( nextPos, colonPos-nextPos );

        nextPos  = colonPos+1;
        colonPos = data.find( SEPARATOR, nextPos );
        if( colonPos == string::npos )
            goto error;

        const string launchTimeout = data.substr( nextPos, colonPos-nextPos );
        this->launchTimeout = atoi( launchTimeout.c_str( ));

        nextPos  = colonPos+1;
        colonPos = data.find( SEPARATOR, nextPos );
        hostname = data.substr( nextPos, colonPos-nextPos );

        switch( this->type )
        {
            case CONNECTIONTYPE_TCPIP:
            {
                nextPos  = colonPos+1;
                colonPos = data.find( SEPARATOR, nextPos );
                if( colonPos != string::npos )
                    goto error;
            
                const string port = data.substr( nextPos, colonPos-nextPos );
                TCPIP.port = atoi( port.c_str( ));
                break;
            }

            default:
                if( colonPos != string::npos )
                    goto error;
        }
    }
    return true;

  error:
    EQWARN << "Could not parse connection description: " << data << endl;
    return false;
}
