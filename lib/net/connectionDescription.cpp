
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
        case TYPE_TCPIP:
            stringStream << "TCPIP";
            break;

        case TYPE_PIPE:
            stringStream << "PIPE";
            break;

        case TYPE_UNI_PIPE:
            stringStream << "UNIPIPE";
            break;
    }        

    stringStream << ":" << bandwidthKBS << ":" << launchCommand << ":" 
                 << launchTimeout << ":" << hostname ;
    
    switch( type )
    {
        case TYPE_TCPIP:
            stringStream << ":" << TCPIP.port;
            break;

        case TYPE_PIPE:
            stringStream << ":" << Pipe.fd;
            break;

        default:
            break;
    }
    return stringStream.str();
}

bool ConnectionDescription::fromString( const string& data )
{
    size_t colonPos = data.find( ':' );
    if( colonPos == string::npos )
        return false;

    const string type = data.substr( 0, colonPos );

    if( type == "TCPIP" )
        this->type = TYPE_TCPIP;
    else if( type == "PIPE" )
        this->type = TYPE_PIPE;
    else if( type == "UNIPIPE" )
        this->type = TYPE_UNI_PIPE;
    else
        return false;

    size_t nextPos = colonPos+1;
    colonPos       = data.find( ':', nextPos );
    if( colonPos == string::npos )
        return false;
    const string bandwidth = data.substr( nextPos, colonPos-nextPos );
    bandwidthKBS = atoi( bandwidth.c_str( ));
    
    nextPos  = colonPos+1;
    colonPos = data.find( ':', nextPos );
    if( colonPos == string::npos )
        return false;
    launchCommand = data.substr( nextPos, colonPos-nextPos );

    nextPos  = colonPos+1;
    colonPos = data.find( ':', nextPos );
    if( colonPos == string::npos )
        return false;
    const string launchTimeout = data.substr( nextPos, colonPos-nextPos );
    this->launchTimeout = atoi( launchTimeout.c_str( ));

    nextPos  = colonPos+1;
    colonPos = data.find( ':', nextPos );
    hostname = data.substr( nextPos, colonPos-nextPos );

    switch( this->type )
    {
        case TYPE_TCPIP:
        {
            nextPos  = colonPos+1;
            colonPos = data.find( ':', nextPos );
            if( colonPos != string::npos )
                return false;
            
            const string port = data.substr( nextPos, colonPos-nextPos );
            TCPIP.port = atoi( port.c_str( ));
            break;
        }

        case TYPE_PIPE:
        {
            nextPos  = colonPos+1;
            colonPos = data.find( ':', nextPos );
            if( colonPos != string::npos )
                return false;
            
            const string port = data.substr( nextPos, colonPos-nextPos );
            Pipe.fd = atoi( port.c_str( ));
            break;
        }

        default:
            if( colonPos != string::npos )
                return false;
    }
    return true;
}
