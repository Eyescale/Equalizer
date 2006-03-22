
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

