
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

using namespace eqs;
using namespace eqBase;
using namespace std;

static Global *_instance = NULL;

Global* Global::instance()
{
    if( !_instance ) 
        _instance = new Global();

    return _instance;
}

Global::Global()
{
    for( int i=0; i<Node::IATTR_ALL; ++i )
        _nodeIAttributes[i] = EQ_NONE;

    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
        _connectionIAttributes[i] = EQ_NONE;

    _connectionIAttributes[ConnectionDescription::IATTR_TYPE] = 
        eqNet::Connection::TYPE_TCPIP;
    _connectionIAttributes[ConnectionDescription::IATTR_TCPIP_PORT] = 0;
    
    _connectionSAttributes[ConnectionDescription::SATTR_HOSTNAME] = "localhost";
}

std::ostream& eqs::operator << ( std::ostream& os, const Global* global )
{
    Global reference;

    os << disableSync;
    os << "global" << endl;
    os << '{' << indent << endl;
#if 0
    for( int i=0; i<Node::IATTR_ALL; ++i )
    {
        if( global->_nodeIAttributes[i] != reference._nodeIAttributes[i] )
        {
        }
    }

    for( int i=0; i<Node::SATTR_ALL; ++i )
    {
        if( global->_nodeSAttributes[i] != reference._nodeSAttributes[i] )
        {
        }
    }
#endif

    for( int i=0; i<ConnectionDescription::IATTR_ALL; ++i )
    {
        const int value = global->_connectionIAttributes[i];
        if( value != reference._connectionIAttributes[i] )
        {

            os << ( i==ConnectionDescription::IATTR_TYPE ?
                    "EQ_CONNECTION_TYPE           " :
                    i==ConnectionDescription::IATTR_TCPIP_PORT ?
                    "EQ_CONNECTION_TCPIP_PORT     " :
                    "EQ_CONNECTION_LAUNCH_TIMEOUT " );
                
            switch( i )
            { 
                case ConnectionDescription::IATTR_TYPE:
                    os << ( value == eqNet::Connection::TYPE_TCPIP ? "TCPIP" : 
                            "PIPE" );
                    break;
                case ConnectionDescription::IATTR_LAUNCH_TIMEOUT:
                    os << value << " ms";
                    break;
                default:
                    os << value;
            }
            os << endl;
        }
    }

    for( int i=0; i<ConnectionDescription::SATTR_ALL; ++i )
    {
        const string& value = global->_connectionSAttributes[i];
        if( value != reference._connectionSAttributes[i] )
        {

            os << ( i==ConnectionDescription::SATTR_HOSTNAME ?
                    "EQ_CONNECTION_HOSTNAME       " :
                    "EQ_CONNECTION_LAUNCH_COMMAND " )
               << "\"" << value << "\"" << endl;
        }
    }

    os << exdent << '}' << endl;
    os << enableSync;
    return os;
}

