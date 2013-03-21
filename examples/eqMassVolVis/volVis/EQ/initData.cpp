
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "initData.h"

namespace massVolVis
{

InitData::InitData()
    : _frameDataId(  lunchbox::UUID::ZERO   )
    , _volumeInfoId( lunchbox::UUID::ZERO   )
    , _windowSystem( eq::WINDOW_SYSTEM_NONE )
    , _filename( "" )
{}


InitData::~InitData()
{
    setFrameDataId(  lunchbox::UUID::ZERO );
    setVolumeInfoId( lunchbox::UUID::ZERO );
}


void InitData::getInstanceData( co::DataOStream& os )
{
    os << _frameDataId << _volumeInfoId << _windowSystem << _filename;
}


void InitData::applyInstanceData( co::DataIStream& is )
{
    is >> _frameDataId >> _volumeInfoId >> _windowSystem >> _filename;

    LBASSERT( _frameDataId != lunchbox::UUID::ZERO );
}


}//namespace massVolVis
