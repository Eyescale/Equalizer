
/*
 * Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 *
 * The init data manages static, per-instance application data. In this
 * example, it holds the model file name, and manages the instanciation of the
 * frame data. The instance data is constructed dynamically (due to the use of a
 * string) and cached for further use. The frame data is instanciated seperately
 * for each thread, so that multiple pipe threads on a node can render different
 * frames concurrently.
 */

#include "initData.h"

using namespace std;
using namespace eqBase;

InitData::InitData()
        : Object( TYPE_INITDATA ),
          _frameDataID( EQ_ID_INVALID ),
          _filename( "rockerArm.ply" ),
          _instanceData( NULL )
{}

InitData::InitData( const void* data, const uint64_t size )
        : Object( TYPE_INITDATA ),
          _instanceData( NULL )
{
    EQASSERT( size > sizeof( _frameDataID ));

    memcpy( &_frameDataID, data, sizeof( _frameDataID ));
    _filename = (char*)(data) + sizeof(_frameDataID);
    EQINFO << "New InitData instance" << endl;
}

InitData::~InitData()
{
    setFrameData( NULL );
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

void InitData::setFrameData( RefPtr<FrameData> frameData )
{
    _clearInstanceData();
    _frameDataID = frameData.isValid() ? frameData->getID() : EQ_ID_INVALID;

    if( _frameData.get( ))
        _frameData->unref();
    frameData.ref();
    _frameData = frameData.get();
}

RefPtr<FrameData> InitData::getFrameData()
{
    if( _frameData.get( ))
        return _frameData.get();
    if( _frameDataID == EQ_ID_INVALID )
        return NULL;

    eqNet::Session* session = getSession();
    EQASSERT( session );

    _frameData = (FrameData*)session->getObject( _frameDataID, 
                                                 Object::SHARE_THREAD );
    _frameData->ref();
    return _frameData.get();
}

void InitData::releaseFrameData()
{
    FrameData* frameData = _frameData.get();
    _frameData = 0;

    if( !frameData )
        return;

    eqNet::Session* session = getSession();
    session->removeRegisteredObject( frameData );
    frameData->unref();
}

void InitData::setFilename( const std::string& filename )
{
    _clearInstanceData();
    _filename = filename;
}
