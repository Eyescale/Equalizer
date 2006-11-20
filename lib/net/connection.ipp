
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

template< typename T >
uint64_t Connection::send( Packet &packet, const std::vector<T>& data ) const
{ 
    if( data.size() == 0 )
        return send( packet );

    if( data.size() == 1 ) // fits in existing packet
    {
        memcpy( (char*)(&packet) + packet.size-sizeof(T), &data[0], sizeof(T) );
        return send( packet );
    }

    // Possible OPT: For big packets, lock the connection and do two send() to
    // avoid memcpy

    uint64_t size   = packet.size + (data.size() - 1) * sizeof(T);
    char*    buffer = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size-sizeof(T) );
    memcpy( buffer + packet.size-sizeof(T), &data[0], data.size() * sizeof(T) );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

template< typename T >
bool Connection::send( const std::vector<T>& receivers, const Packet& packet )
{
    if( receivers.empty( ))
        return true;

    const int nReceivers = receivers.size();
    for( int i=0; i<nReceivers; ++i )
        if( receivers[i]->getConnection()->send( &packet, packet.size ) != 
            packet.size )

            return false;

    return true;
}

template< typename T >
bool Connection::send( const std::vector<T>& receivers, Packet& packet,
                       const void* data, const uint64_t dataSize )
{
    if( receivers.empty( ))
        return true;

    if( dataSize <= 8 ) // fits in existing packet
    {
        if( dataSize != 0 )
            memcpy( (char*)(&packet) + packet.size-8, data, dataSize );
        return send<T>( receivers, packet );
    }

    uint64_t       size   = packet.size-8 + dataSize;
    size += (4 - size%4);
    char*          buffer = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size-8 );
    memcpy( buffer + packet.size-8, data, dataSize );

    ((Packet*)buffer)->size = size;

    const int nReceivers = receivers.size();
    for( int i=0; i<nReceivers; ++i )
        if( receivers[i]->getConnection()->send( buffer, size ) != size )
            return false;

    return true;
}

