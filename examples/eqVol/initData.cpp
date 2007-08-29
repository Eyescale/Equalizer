/*
 * Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

using namespace eqBase;
using namespace std;

namespace eqVol
{

InitData::InitData()
: _dataFilename( "Bucky32x32x32.raw" )
,_windowSystem( eq::WINDOW_SYSTEM_NONE )
,_instanceData( 0 )
{}

InitData::~InitData()
{
    setFrameDataID( EQ_ID_INVALID );
}


const void* InitData::getInstanceData( uint64_t* size )
{
	*size = sizeof( uint32_t ) + sizeof( eq::WindowSystem ) + _dataFilename.length() + 1;// + _infoFilename.length() + 1;
    if( _instanceData )
        return _instanceData;

    _instanceData = new char[ *size ];

    reinterpret_cast< uint32_t* >( _instanceData )[0] =  _frameDataID;
    reinterpret_cast< uint32_t* >( _instanceData )[1] =  _windowSystem;

    const char* string = _dataFilename.c_str();
    memcpy( _instanceData + 2*sizeof( uint32_t ), string, _dataFilename.length()+1 );
 
//    string = _infoFilename.c_str();
//    memcpy( _instanceData + sizeof( uint32_t ) + _dataFilename.length()+1, string, _infoFilename.length()+1 );

    return _instanceData;
}


void InitData::applyInstanceData( const void* data, const uint64_t size )
{
    EQASSERT( size > sizeof( _frameDataID ));

    _frameDataID  = reinterpret_cast< const uint32_t* >( data )[0];
    _windowSystem = reinterpret_cast< const eq::WindowSystem* >( data )[1];
    _dataFilename = static_cast<const char*>( data ) + 2*sizeof( uint32_t );
//    _infoFilename = static_cast<const char*>( data ) + sizeof( uint32_t ) + _dataFilename.length()+1;

    EQASSERT( _frameDataID != EQ_ID_INVALID );

    EQINFO << "New InitData instance" << endl;
}


void InitData::_clearInstanceData()
{
    delete [] _instanceData;
    _instanceData = 0;
}
}
