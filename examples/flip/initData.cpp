
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "initData.h"

using namespace std;
using namespace eqBase;

InitData::InitData()
        : Object( TYPE_INITDATA, eqNet::CMD_OBJECT_CUSTOM ),
          _frameDataID( EQ_INVALID_ID ),
          _filename( "rockerArm.ply" ),
          _instanceData( NULL )
{}

InitData::InitData( const void* data, const uint64_t size )
        : Object( TYPE_INITDATA, eqNet::CMD_OBJECT_CUSTOM ),
          _instanceData( NULL )
{
    EQASSERT( size > sizeof( _frameDataID ));

    memcpy( &_frameDataID, data, sizeof( _frameDataID ));
    _filename = (char*)(data) + sizeof(_frameDataID);
    EQINFO << "New InitData instance" << endl;
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

void InitData::setFrameData( FrameData* frameData )
{
    _clearInstanceData();
    _frameDataID = frameData ? frameData->getID() : EQ_INVALID_ID;
    _frameData   = frameData;
}

FrameData* InitData::getFrameData()
{
    if( _frameData.isValid( ))
        return _frameData.get();
    if( _frameDataID == EQ_INVALID_ID )
        return NULL;

    eqNet::Session* session = getSession();
    EQASSERT( session );

    _frameData = (FrameData*)session->getObject( _frameDataID );
    return _frameData.get();
}

void InitData::setFilename( const std::string& filename )
{
    _clearInstanceData();
    _filename = filename;
    EQERROR << _filename << " == " << filename << endl;
    exit( 0 );
}
