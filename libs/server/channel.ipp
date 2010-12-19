
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

namespace eq
{
namespace server
{

template< typename T >
void Channel::send( co::ObjectPacket &packet, const std::vector<T>& data )
{ 
    packet.objectID = getID(); 
    getNode()->send( packet, data ); 
}

}
}
