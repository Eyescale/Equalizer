
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

template< typename T >
bool Connection::send( Packet &packet, const std::vector<T>& data ) const
{ 
    if( data.size() == 0 )
        return send( packet );

    size_t       packetStorage = MAX( 8, sizeof( T ));
    const size_t offset        = packetStorage % 8;
    if( offset )
        packetStorage += 8 - offset;
    const size_t nItems        = data.size();
    const size_t dataSize      = nItems * sizeof( T );

    if( dataSize <= packetStorage ) // fits in existing packet
    {
        memcpy( (char*)(&packet) + packet.size-packetStorage, &data[0], 
                dataSize );
        return send( packet );
    }

    // Possible OPT: For big packets, lock the connection and do two send()
    // calls to avoid memcpy

    uint64_t size   = packet.size - packetStorage + dataSize;
    char*    buffer = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size - packetStorage );
    memcpy( buffer + packet.size - packetStorage, &data[0], dataSize );

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

