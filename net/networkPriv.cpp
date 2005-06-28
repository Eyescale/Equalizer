
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "networkPriv.h"

#include "connectionDescription.h"
#include "global.h"
#include "network.h"
#include "pipeNetwork.h"
#include "socketNetwork.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

Network::Network( const uint id, Session* session )
        : eqNet::Network(id),
          _session(session)
{}

Network::~Network()
{
    // TODO: ConnectionDescription cleanup
}

Network* Network::create( const uint id, Session* session, 
    const eqNet::NetworkProtocol protocol )
{
    switch( protocol )
    {
        case eqNet::PROTO_TCPIP:
            return new SocketNetwork( id, session );

        case eqNet::PROTO_PIPE:
            return new PipeNetwork( id, session );

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }
}

void Network::addNode( const uint nodeID,
    const eqNet::ConnectionDescription& description )
{
    ConnectionDescription *desc = new ConnectionDescription();
    *desc = description;

    if( typeid(this) == typeid( SocketNetwork* ))
    {
        if( description.parameters.TCPIP.address != NULL)
            desc->parameters.TCPIP.address = 
                strdup( description.parameters.TCPIP.address );
    }
    else if( typeid(this) == typeid( PipeNetwork* ))
    {
        if( description.parameters.PIPE.entryFunc != NULL)
            desc->parameters.PIPE.entryFunc =
                strdup( description.parameters.PIPE.entryFunc );
    }
   
    _descriptions[nodeID] = desc;
}


const char* Network::_createLaunchCommand( const uint nodeID )
{
    IDHash<ConnectionDescription*>::iterator iter = _descriptions.find( nodeID);

    if( iter == _descriptions.end() )
        return NULL;

    ConnectionDescription* description = (*iter).second;
    const char*          launchCommand = description->launchCommand;
    if( !launchCommand )
        return NULL;

    const size_t      launchCommandLen = strlen( launchCommand );
    size_t                  resultSize = 256;
    char*                       result = (char*)alloca(resultSize);
    size_t                 resultIndex = 0;

    for( size_t i=0; i<launchCommandLen-1; i++ )
    {
        if( launchCommand[i] == '%' )
        {
            const char* replacement = NULL;
            switch( launchCommand[i+1] )
            {
                case 'c':
                    replacement = Global::getProgramName();
                    break;
            }

            if( replacement )
            {
                // check string length
                const size_t replacementLen = strlen( replacement );
                size_t newSize = resultSize;
                
                while( newSize < i + replacementLen )
                    newSize << 1;
                if( newSize > resultSize )
                {
                    char* newResult = (char*)alloca(newSize);
                    memcpy( newResult, result, resultSize );
                    result     = newResult;
                    resultSize = newSize;
                }

                // replace
                memcpy( &result[resultIndex], replacement, replacementLen );
                resultIndex += replacementLen;
                i++;
            }
        }
        else
        {
            result[resultIndex++] = launchCommand[i];
            if( resultIndex == resultSize ) // check string length
            {
                resultSize      = resultSize << 1;
                char* newResult = (char*)alloca(resultSize);
                memcpy( newResult, result, resultSize >> 1 );
                result = newResult;
            }
        }
    }

    result[resultIndex] = '\0';
    return strdup(result);
}
