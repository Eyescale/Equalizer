
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

template< class T >
uint64_t Connection::send( Packet &packet, const std::vector<T>& data ) const
{ 
    if( data.size() == 0 )
        return send( packet );

    if( data.size() == 1 ) // fits in existing packet
    {
        memcpy( (char*)(&packet) + packet.size-sizeof(T), &data[0], sizeof(T) );
        return send( packet );
    }

    uint64_t size   = packet.size + (data.size() - 1) * sizeof(T);
    char*    buffer = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size-sizeof(T) );
    memcpy( buffer + packet.size-sizeof(T), &data[0], data.size() * sizeof(T) );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

