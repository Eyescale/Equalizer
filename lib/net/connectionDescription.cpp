
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connectionDescription.h"

#include <sstream>

using namespace eqNet;
using namespace std;

#define SEPARATOR '#'

string ConnectionDescription::toString() const
{
    ostringstream description;
    serialize( description );
    return description.str();
}

void ConnectionDescription::serialize( std::ostream& os ) const
{
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
            os << "TCPIP";
            break;

        case CONNECTIONTYPE_SDP:
            os << "SDP";
            break;

        case CONNECTIONTYPE_PIPE:
            os << "PIPE";
            break;
    }        

    os << SEPARATOR << bandwidthKBS << SEPARATOR << _launchCommand 
       << SEPARATOR << launchTimeout << SEPARATOR << _hostname ;
    
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
        case CONNECTIONTYPE_SDP:
            os << SEPARATOR << TCPIP.port;
            break;

        default:
            break;
    }
    os << SEPARATOR;
}

bool ConnectionDescription::fromString( string& data )
{
    {
        size_t nextPos = data.find( SEPARATOR );
        // assume hostname[:port[:type]|:type] format
        if( nextPos == string::npos )
        {
            type     = CONNECTIONTYPE_TCPIP;
            nextPos = data.find( ':' );
            if( nextPos == string::npos ) // assume hostname format
            {
                _hostname = data;
                data.clear();
                return true;
            }

            _hostname = data.substr( 0, nextPos );
            data      = data.substr( nextPos + 1 );

            while( nextPos != string::npos )
            {
                nextPos            = data.find( ':' );
                const string token = data.substr( 0, nextPos );
                data               = data.substr( nextPos + 1 );
                
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

        const string type = data.substr( 0, nextPos );
        data              = data.substr( nextPos + 1 );

        if( type == "TCPIP" )
            this->type = CONNECTIONTYPE_TCPIP;
        else if( type == "SDP" )
            this->type = CONNECTIONTYPE_SDP;
        else if( type == "PIPE" )
            this->type = CONNECTIONTYPE_PIPE;
        else
            goto error;

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        const string bandwidth = data.substr( 0, nextPos );
        data                   = data.substr( nextPos + 1 );
        bandwidthKBS = atoi( bandwidth.c_str( ));
    
        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;
        _launchCommand = data.substr( 0, nextPos );
        data           = data.substr( nextPos + 1 );

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        const string launchTimeout = data.substr( 0, nextPos );
        data                       = data.substr( nextPos + 1 );
        this->launchTimeout = atoi( launchTimeout.c_str( ));

        nextPos = data.find( SEPARATOR );
        if( nextPos == string::npos )
            goto error;

        _hostname = data.substr( 0, nextPos );
        data      = data.substr( nextPos + 1 );

        switch( this->type )
        {
            case CONNECTIONTYPE_TCPIP:
            case CONNECTIONTYPE_SDP:
            {
                nextPos = data.find( SEPARATOR );
                if( nextPos == string::npos )
                    goto error;
            
                const string port = data.substr( 0, nextPos );
                data              = data.substr( nextPos + 1 );
                TCPIP.port        = atoi( port.c_str( ));
                break;
            }

            default:
                break;
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
