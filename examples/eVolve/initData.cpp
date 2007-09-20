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

namespace eVolve
{

InitData::InitData()
        : _frameDataID(  EQ_UNDEFINED_UINT32 )
        , _windowSystem( eq::WINDOW_SYSTEM_NONE )
        , _useCg       ( 1 )
#ifdef WIN32_VC
        , _dataFilename( "../examples/eVolve/Bucky32x32x32_d.raw" )
#else
        , _dataFilename( "Bucky32x32x32_d.raw" )
#endif
        , _instanceData( 0 )
{}

InitData::~InitData()
{
    setFrameDataID( EQ_ID_INVALID );
}

bool InitData::getUseCgSwitch()  const
{ 
#ifdef CG_INSTALLED
    return _useCg;
#else
    return false;
#endif
}


const void* InitData::getInstanceData( uint64_t* size )
{
    *size = sizeof( uint32_t         ) + 
            sizeof( eq::WindowSystem ) +
            sizeof( uint32_t         ) +
            _dataFilename.length() + 1;

    if( _instanceData )
        return _instanceData;

    _instanceData = new char[ *size ];

    reinterpret_cast< uint32_t* >( _instanceData )[0] = _frameDataID;
    reinterpret_cast< uint32_t* >( _instanceData )[1] = _windowSystem;
    reinterpret_cast< uint32_t* >( _instanceData )[2] = _useCg;

    const char* string = _dataFilename.c_str();
    memcpy( _instanceData + 3*sizeof( uint32_t ), 
            string, 
            _dataFilename.length()+1 );
 
    return _instanceData;
}

void InitData::applyInstanceData( const void* data, const uint64_t size )
{
    EQASSERT( size > ( sizeof( _frameDataID ) + sizeof( _windowSystem )));

    _frameDataID  = reinterpret_cast< const uint32_t*         >( data )[0];
    _windowSystem = reinterpret_cast< const eq::WindowSystem* >( data )[1];
    _useCg        = reinterpret_cast< const uint32_t*         >( data )[2];
    _dataFilename = static_cast<const char*>( data ) + 
                    3*sizeof( uint32_t );

    EQASSERT( _frameDataID != EQ_ID_INVALID );

    EQINFO << "New InitData instance" << endl;
}


void InitData::_clearInstanceData()
{
    delete [] _instanceData;
    _instanceData = 0;
}
}
