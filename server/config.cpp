
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "node.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Config::_construct()
{
    _latency     = 1;
    _frameNumber = 0;

    registerCommand( eq::CMD_CONFIG_INIT, this, reinterpret_cast<CommandFcn>(
                         &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_INIT, this, reinterpret_cast<CommandFcn>(
                         &eqs::Config::_reqInit ));
    registerCommand( eq::CMD_CONFIG_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eqs::Config::_reqExit ));
    registerCommand( eq::CMD_CONFIG_FRAME_BEGIN, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_FRAME_BEGIN, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Config::_reqFrameBegin ));
    registerCommand( eq::CMD_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_cmdRequest ));
    registerCommand( eq::REQ_CONFIG_FRAME_END, this,
                     reinterpret_cast<CommandFcn>( &eqs::Config::_reqFrameEnd));

    EQINFO << "New config @" << (void*)this << endl;
}

Config::Config()
        : eqNet::Session( eq::CMD_CONFIG_ALL )
{
    _construct();
}


struct ReplaceChannelData
{
    Channel* oldChannel;
    Channel* newChannel;
};
static TraverseResult replaceChannelCB(Compound* compound, void* userData )
{
    ReplaceChannelData* data = (ReplaceChannelData*)userData;
                            
    if( compound->getChannel() == data->oldChannel )
        compound->setChannel( data->newChannel );
    
    return TRAVERSE_CONTINUE;
}

Config::Config( const Config& from )
        : eqNet::Session( eq::CMD_CONFIG_ALL ),
          _server( from._server )
{
    _construct();

    const uint32_t nCompounds = from.nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound      = from.getCompound(i);
        Compound* compoundClone = new Compound( *compound );

        addCompound( compoundClone );
        // channel is replaced below
    }

    const uint32_t nNodes = from.nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        eqs::Node* node      = from.getNode(i);
        eqs::Node* nodeClone = new eqs::Node( *node );
        
        addNode( nodeClone );

        // replace channels in compounds
        const uint32_t nPipes = node->nPipes();
        for( uint32_t j=0; j<nPipes; j++ )
        {
            Pipe* pipe      = node->getPipe(j);
            Pipe* pipeClone = nodeClone->getPipe(j);
            
            const uint32_t nWindows = pipe->nWindows();
            for( uint32_t k=0; k<nWindows; k++ )
            {
                Window* window      = pipe->getWindow(k);
                Window* windowClone = pipeClone->getWindow(k);
            
                const uint32_t nChannels = window->nChannels();
                for( uint32_t l=0; l<nChannels; l++ )
                {
                    Channel* channel      = window->getChannel(l);
                    Channel* channelClone = windowClone->getChannel(l);

                    ReplaceChannelData data;
                    data.oldChannel = channel;
                    data.newChannel = channelClone;

                    for( uint32_t m=0; m<nCompounds; m++ )
                    {
                        Compound* compound = getCompound(m);

                        Compound::traverse( compound, replaceChannelCB, 
                                            replaceChannelCB, NULL, &data );
                    }
                }
            }
        }
    }
}

void Config::addNode( Node* node )
{
    _nodes.push_back( node ); 

    node->_config = this; 
    node->adjustLatency( _latency );
}

bool Config::removeNode( Node* node )
{
    vector<Node*>::iterator iter = _nodes.begin();
    for( ; iter != _nodes.end(); ++iter )
        if( (*iter) == node )
            break;

    if( iter == _nodes.end( ))
        return false;

    _nodes.erase( iter );

    node->adjustLatency( -_latency );
    node->_config = NULL; 

    return true;
}

void Config::setLatency( const uint32_t latency )
{
    if( _latency == latency )
        return;

    const int delta = latency - _latency;
    for( vector<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         ++iter )

        (*iter)->adjustLatency( delta );
}

// pushes the request to the main thread to be handled asynchronously
eqNet::CommandResult Config::_cmdRequest( eqNet::Node* node,
                                          const eqNet::Packet* packet )
{
    _server->pushRequest( node, packet );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqInit( eqNet::Node* node,
                                       const eqNet::Packet* pkg )
{
    const eq::ConfigInitPacket* packet = (eq::ConfigInitPacket*)pkg;
    eq::ConfigInitReplyPacket   reply( packet );
    EQINFO << "handle config init " << packet << endl;

    reply.result = _init( packet->initID );
    EQINFO << "config init result: " << reply.result << endl;
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqExit( eqNet::Node* node, 
                                       const eqNet::Packet* pkg )
{
    const eq::ConfigExitPacket* packet = (eq::ConfigExitPacket*)pkg;
    eq::ConfigExitReplyPacket   reply( packet );
    EQINFO << "handle config exit " << packet << endl;

    reply.result = _exit();
    EQINFO << "config exit result: " << reply.result << endl;
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqFrameBegin( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    const eq::ConfigFrameBeginPacket* packet = (eq::ConfigFrameBeginPacket*)pkg;
    eq::ConfigFrameBeginReplyPacket   reply( packet );
    EQVERB << "handle config frame begin " << packet << endl;

    reply.result = _frameBegin( packet->frameID );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqFrameEnd( eqNet::Node* node, 
                                           const eqNet::Packet* pkg )
{
    const eq::ConfigFrameEndPacket* packet = (eq::ConfigFrameEndPacket*)pkg;
    eq::ConfigFrameEndReplyPacket   reply( packet );
    EQVERB << "handle config frame end " << packet << endl;

    reply.result = _frameEnd();
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}


//===========================================================================
// operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
bool Config::_init( const uint32_t initID )
{
    _frameNumber = 0;

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->init();
    }

    if( !_initConnectNodes()  ||
        !_initNodes( initID ) ||
        !_initPipes( initID ) )
    {
        _exit();
        return false;
    }
    return true;
}

bool Config::_initConnectNodes()
{
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;

        if( !node->initConnect( ))
        {
            EQERROR << "Connection to " << node << " failed." << endl;
            return false;
        }
    }
    return true;
}

bool Config::_initNodes( const uint32_t initID )
{
    const string&              name    = getName();
    bool                       success = true;

    eq::NodeCreateConfigPacket createConfigPacket;
    createConfigPacket.configID        = _id;

    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        if( !node->syncConnect( ))
        {
            EQERROR << "Connection of node " << (void*)node << " failed." 
                    << endl;
            success = false;
        }
        
        if( !success ) 
            continue;

        // initialize nodes
        node->send( createConfigPacket, name );
        registerObject( node, _server.get( ));
        node->startInit( initID );
    }

    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        if( !node->syncInit( ))
        {
            EQERROR << "Init of node " << (void*)node << " failed." 
                    << endl;
            success = false;
        }
    }
    return success;
}

bool Config::_initPipes( const uint32_t initID )
{
    // start pipe-window-channel init in parallel
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        eq::NodeCreatePipePacket createPipePacket( getID(), node->getID( ));

        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( !pipe->isUsed( ))
                continue;
            
            registerObject( pipe, _server.get( ));
            createPipePacket.pipeID = pipe->getID();
            node->send( createPipePacket );
            pipe->startInit( initID ); // recurses down
        }
    }

    // sync init of all pipe-window-channel entities
    bool success = true;
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( !pipe->isUsed( ))
                continue;

            if( !pipe->syncInit( ))
                success = false;
        }
    }
    return success;
}

bool Config::_exit()
{
//     foreach compound
//         call exit compound cb's
//     foreach compound
//         sync exit

    const bool cleanExit = ( _exitPipes() &&
                             _exitNodes()  );

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->exit();
    }

    _frameNumber = 0;
    return cleanExit;
}

bool Config::_exitPipes()
{
    // start pipe-window-channel exit in parallel
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;

        if( !node->isUsed( ))
            continue;
            
        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( pipe->getState() == Pipe::STATE_STOPPED )
                continue;

            pipe->startExit();
        }
    }

    // sync exit of all pipe-window-channel entities
    bool success = true;
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( !node->isUsed( ))
            continue;
        
        eq::NodeDestroyPipePacket destroyPipePacket( getID(), node->getID( ));

        const vector<Pipe*>& pipes = node->getPipes();
        for( PipeIter iter = pipes.begin(); iter != pipes.end(); ++iter )
        {
            Pipe* pipe = *iter;
            if( pipe->getState() != Pipe::STATE_STOPPING )
                continue;

            if( !pipe->syncExit( ))
                success = false;

            destroyPipePacket.pipeID = pipe->getID();
            send( destroyPipePacket );
            deregisterObject( pipe );
        }
    }

    return success;
}

bool Config::_exitNodes()
{
    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( node->getState() == eqNet::Node::STATE_STOPPED )
            continue;

        node->startExit();
    }

    bool success = true;

    for( NodeIter iter = _nodes.begin(); iter != _nodes.end(); ++iter )
    {
        Node* node = *iter;
        if( node->getState() == Node::STATE_STOPPED )
            continue;

        if( !node->syncExit( ))
        {
            EQERROR << "Exit of " << node << " failed." << endl;
            success = false;
        }

        node->stop();
        deregisterObject( node );
    }
    return success;
}


uint32_t Config::_frameBegin( const uint32_t frameID )
{
    ++_frameNumber;

    const uint32_t nCompounds = this->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
    {
        Compound* compound = getCompound( i );
        compound->update();
    }

    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->update( frameID );
    }
    
    return _frameNumber;
}

uint32_t Config::_frameEnd()
{
    const uint32_t nNodes = this->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
    {
        Node* node = getNode( i );
        if( node->isUsed( ))
            node->syncUpdate();
    }

    return (_frameNumber > _latency ? _frameNumber-_latency : 0);
}


ostream& eqs::operator << ( ostream& os, const Config* config )
{
    if( !config )
        return os;
    
    os << disableFlush << disableHeader << "config " << endl;
    os << "{" << endl << indent;

    const uint32_t nNodes = config->nNodes();
    for( uint32_t i=0; i<nNodes; ++i )
        os << config->getNode(i);

    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; ++i )
        os << config->getCompound(i);
    
    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}
