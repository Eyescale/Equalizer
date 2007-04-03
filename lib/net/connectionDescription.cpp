
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

        case CONNECTIONTYPE_SDP:
            description << "SDP";
            break;

        case CONNECTIONTYPE_PIPE:
            description << "PIPE";
            break;
    }        

    description << SEPARATOR << bandwidthKBS << SEPARATOR << _launchCommand 
                << SEPARATOR << launchTimeout << SEPARATOR << _hostname ;
    
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
        case CONNECTIONTYPE_SDP:
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
        // assume hostname[:port[:type]|:type] format
        if( colonPos == string::npos )
        {
            type     = CONNECTIONTYPE_TCPIP;
            colonPos = data.find( ':' );
            if( colonPos == string::npos ) // assume hostname format
            {
                _hostname = data;
                return true;
            }

            _hostname       = data.substr( 0, colonPos );

            while( colonPos != string::npos )
            {
                size_t     nextPos = colonPos+1;
                colonPos           = data.find( ':', nextPos );
                const string token = data.substr( nextPos, colonPos - nextPos );

                if( !token.empty() && isdigit( token[0] )) // port
                    TCPIP.port = atoi( token.c_str( ));
                else if( token == "TCPIP" )
                    type = CONNECTIONTYPE_TCPIP;
                else if( token == "SDP" )
                    type = CONNECTIONTYPE_SDP;
                else if( token == "PIPE" )
                    type = CONNECTIONTYPE_PIPE;
                else
                    goto error;
            }
            return true;
        }

        const string type = data.substr( 0, colonPos );

        if( type == "TCPIP" )
            this->type = CONNECTIONTYPE_TCPIP;
        else if( type == "SDP" )
            this->type = CONNECTIONTYPE_SDP;
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
        _launchCommand = data.substr( nextPos, colonPos-nextPos );

        nextPos  = colonPos+1;
        colonPos = data.find( SEPARATOR, nextPos );
        if( colonPos == string::npos )
            goto error;

        const string launchTimeout = data.substr( nextPos, colonPos-nextPos );
        this->launchTimeout = atoi( launchTimeout.c_str( ));

        nextPos  = colonPos+1;
        colonPos = data.find( SEPARATOR, nextPos );
        _hostname = data.substr( nextPos, colonPos-nextPos );

        switch( this->type )
        {
            case CONNECTIONTYPE_TCPIP:
            case CONNECTIONTYPE_SDP:
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

void ConnectionDescription::setHostname( const std::string& hostname )
{
    _hostname = hostname;
}

const string& ConnectionDescription::getHostname() const
{
    return _hostname;
}

void ConnectionDescription::setLaunchCommand( const std::string& launchCommand )
{
    _launchCommand = launchCommand;
}

const string& ConnectionDescription::getLaunchCommand() const
{
    return _launchCommand;
}
