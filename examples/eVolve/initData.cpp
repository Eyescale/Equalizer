
/*
 * Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 
 *
 * The init data manages static, per-instance application data. In this example,
 * it holds the model file name, and manages the instantiation of the frame
 * data. The frame data is instantiated seperately for each (pipe) thread, so
 * that multiple pipe threads on a node can render different frames
 * concurrently.
 */

#include "initData.h"

using namespace eq::base;
using namespace std;

namespace eVolve
{

InitData::InitData()
        : _frameDataID(  EQ_UNDEFINED_UINT32 )
        , _windowSystem( eq::WINDOW_SYSTEM_NONE )
        , _precision( 2 )
        , _brightness( 1.0f )
        , _alpha( 1.0f )
#ifdef WIN32_VC
        , _filename( "../examples/eVolve/Bucky32x32x32_d.raw" )
#else
        , _filename( "../share/data/Bucky32x32x32_d.raw" )
#endif
{}

InitData::~InitData()
{
    setFrameDataID( EQ_ID_INVALID );
}

void InitData::getInstanceData( eq::net::DataOStream& os )
{
    os << _frameDataID << _windowSystem << _precision << _brightness << _alpha
       << _filename;
}

void InitData::applyInstanceData( eq::net::DataIStream& is )
{
    is >> _frameDataID >> _windowSystem >> _precision >> _brightness >> _alpha
       >> _filename;

    EQASSERT( _frameDataID != EQ_ID_INVALID );
    EQINFO << "New InitData instance" << endl;
}
}
