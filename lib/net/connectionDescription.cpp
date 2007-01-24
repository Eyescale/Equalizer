
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

#include <sstream>

using namespace eqNet;
using namespace std;

string ConnectionDescription::toString()
{
    ostringstream stringStream;

    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
            stringStream << "TCPIP";
            break;

        case CONNECTIONTYPE_PIPE:
            stringStream << "PIPE";
            break;
    }        

    stringStream << ":" << bandwidthKBS << ":" << launchCommand << ":" 
                 << launchTimeout << ":" << hostname ;
    
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
            stringStream << ":" << TCPIP.port;
            break;

        default:
            break;
    }
    return stringStream.str();
}

bool ConnectionDescription::fromString( const string& data )
{
    {
        size_t colonPos = data.find( ':' );
        if( colonPos == string::npos )
            goto error;

        const string type = data.substr( 0, colonPos );

        if( type == "TCPIP" )
            this->type = CONNECTIONTYPE_TCPIP;
        else if( type == "PIPE" )
            this->type = CONNECTIONTYPE_PIPE;
        else
            goto error;

        size_t nextPos = colonPos+1;
        colonPos       = data.find( ':', nextPos );
        if( colonPos == string::npos )
            goto error;
        const string bandwidth = data.substr( nextPos, colonPos-nextPos );
        bandwidthKBS = atoi( bandwidth.c_str( ));
    
        nextPos  = colonPos+1;
        colonPos = data.find( ':', nextPos );
        if( colonPos == string::npos )
            goto error;
        launchCommand = data.substr( nextPos, colonPos-nextPos );

        nextPos  = colonPos+1;
        colonPos = data.find( ':', nextPos );
        if( colonPos == string::npos )
            goto error;

        const string launchTimeout = data.substr( nextPos, colonPos-nextPos );
        this->launchTimeout = atoi( launchTimeout.c_str( ));

        nextPos  = colonPos+1;
        colonPos = data.find( ':', nextPos );
        hostname = data.substr( nextPos, colonPos-nextPos );

        switch( this->type )
        {
            case CONNECTIONTYPE_TCPIP:
            {
                nextPos  = colonPos+1;
                colonPos = data.find( ':', nextPos );
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
