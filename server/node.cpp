
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "pipe.h"

using namespace eqs;


Node::Node()
        : eqNet::Node(),
          _used(0),
          _config(NULL)
{
    _autoLaunch = true;
}

void Node::addPipe( Pipe* pipe )
{
    _pipes.push_back( pipe ); 
    pipe->_node = this; 
}

//===========================================================================
// Operations
//===========================================================================

void Node::sendInit()
{
}

bool Node::syncInit()
{
}

std::ostream& eqs::operator << ( std::ostream& os, const Node* node )
{
    if( !node )
    {
        os << "NULL node";
        return os;
    }
    
    const uint nPipes = node->nPipes();
    os << "node " << (void*)node << ( node->isUsed() ? " used " : " unused " )
       << nPipes << " pipes";
    for( uint i=0; i<nPipes; i++ )
        os << std::endl << "    " << node->getPipe(i);

    return os;
}
