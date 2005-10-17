
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

//#include "channel.h"
#include "node.h"
#include "window.h"

using namespace eqs;


Pipe::Pipe()
        : _used( 0 ),
          _node( NULL )
{}

void Pipe::addWindow( Window* window )
{
    _windows.push_back( window ); 
    window->_pipe = this; 
}

void Pipe::refUsed()
{
    _used++;
    if( _node ) 
        _node->refUsed(); 
}
void Pipe::unrefUsed()
{
    _used--;
    if( _node ) 
        _node->unrefUsed(); 
}

std::ostream& eqs::operator << ( std::ostream& os, const Pipe* pipe )
{
    if( !pipe )
    {
        os << "NULL pipe";
        return os;
    }
    
    const uint nWindows = pipe->nWindows();
    os << "pipe " << (void*)pipe << ( pipe->isUsed() ? " used " : " unused " )
       << nWindows << " windows";
    
    for( uint i=0; i<nWindows; i++ )
        os << std::endl << "    " << pipe->getWindow(i);
    
    return os;
}
