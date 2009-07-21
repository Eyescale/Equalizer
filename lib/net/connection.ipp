
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#define ASSEMBLE_THRESHOLD (4096)

template< typename T >
bool Connection::send( Packet &packet, const std::vector<T>& data )
{ 
    if( data.size() == 0 )
        return send( packet );

    size_t       packetStorage = EQ_MAX( 8, sizeof( T ));
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

    const uint64_t headerSize  = packet.size - packetStorage;
    const uint64_t size        = headerSize + dataSize;
    if( size > ASSEMBLE_THRESHOLD )
    {
        // OPT: lock the connection and use two send() to avoid big memcpy
        packet.size = size;

        lockSend();
        const bool ret = ( send( &packet,  headerSize, true ) &&
                           send( &data[0], dataSize,   true ));
        unlockSend();
        return ret;
    }
    // else

    char* buffer = static_cast<char*>( alloca( size ));

    memcpy( buffer,              &packet,  headerSize );
    memcpy( buffer + headerSize, &data[0], dataSize );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}
