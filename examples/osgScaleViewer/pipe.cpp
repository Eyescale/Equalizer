
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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
 */

#include "pipe.h"

#include "config.h"

namespace osgScaleViewer
{

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent )
{
}

Pipe::~Pipe()
{
}

const FrameData& Pipe::getFrameData() const
{
    return _frameData;
}

bool Pipe::configInit( const uint32_t initID )
{
    if( !eq::Pipe::configInit( initID ))
        return false;

    Config* config = static_cast<Config*>( getConfig( ));
    const InitData& initData = config->getInitData();
    const uint32_t frameDataID = initData.getFrameDataID();
    const bool mapped = config->mapObject( &_frameData, frameDataID );
    EQASSERT( mapped );

    return mapped;
}

bool Pipe::configExit()
{
    getConfig()->unmapObject( &_frameData );
    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID,
                       const uint32_t frameNumber )
{
    _frameData.sync( frameID );
    eq::Pipe::frameStart( frameID, frameNumber );
}

}
