
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "initData.h"

InitData::InitData()
        : Mobject( OBJECT_INITDATA ),
          _frameDataID( EQ_INVALID_ID ),
          _instanceData( NULL )
{}

InitData::InitData( const void* data, const uint64_t size )
        : Mobject( OBJECT_INITDATA ),
          _instanceData( NULL )
{
    EQASSERT( size > sizeof( _frameDataID ));

    memcpy( &_frameDataID, data, sizeof( _frameDataID ));
    _filename = (char*)(data) + sizeof(_frameDataID);
}

const void* InitData::getInstanceData( uint64_t* size )
{
    *size = sizeof( _frameDataID ) + _filename.length() + 1;
    if( _instanceData )
        return _instanceData;

    _instanceData = (char*)malloc( *size );
    memcpy( _instanceData, &_frameDataID, sizeof( _frameDataID ));

    const char* string = _filename.c_str();
    memcpy( _instanceData + sizeof( _frameDataID ), string, 
            _filename.length()+1 );

    return _instanceData;
}

void InitData::_clearInstanceData()
{
    if( !_instanceData )
        return;

    free( _instanceData );
    _instanceData = NULL;
}

void InitData::setFrameData( const FrameData* frameData )
{
    _clearInstanceData();
    _frameDataID = frameData ? frameData->getID() : EQ_INVALID_ID;
}

FrameData* InitData::getFrameData()
{
    if( _frameDataID == EQ_INVALID_ID )
        return NULL;

    eqNet::Session* session = getSession();
    if( !session )
        return NULL;

    return (FrameData*)session->getMobject( _frameDataID );
}

void InitData::setFilename( const std::string& filename )
{
    _clearInstanceData();
    _filename = filename;
}

