/*
 * Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved. 
 *
 * The init data manages static, per-instance application data. In this example,
 * it holds the model file name, and manages the instantiation of the frame
 * data. The frame data is instantiated seperately for each (pipe) thread, so
 * that multiple pipe threads on a node can render different frames
 * concurrently.
 */

#include "initData.h"

using namespace eqBase;
using namespace std;

namespace eqPly
{

InitData::InitData()
        : _frameDataID( EQ_ID_INVALID )
        , _modelID( EQ_ID_INVALID )
        , _windowSystem( eq::WINDOW_SYSTEM_NONE )
        , _renderMode( mesh::RENDER_MODE_DISPLAY_LIST )
        , _useGLSL( false )
        , _invFaces( false )
{}

InitData::~InitData()
{
    setFrameDataID( EQ_ID_INVALID );
}

void InitData::getInstanceData( eqNet::DataOStream& os )
{
    os << _frameDataID << _modelID << _windowSystem << _renderMode << _useGLSL
       << _invFaces ;
}

void InitData::applyInstanceData( eqNet::DataIStream& is )
{
    is >> _frameDataID >> _modelID >> _windowSystem >> _renderMode >> _useGLSL
       >> _invFaces;

    EQASSERT( _frameDataID != EQ_ID_INVALID );
    EQINFO << "New InitData instance" << endl;
}
}
