
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "frame.h"

#include "compound.h"
#include "frameData.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

using namespace co::base;
using namespace std;

namespace eq
{
namespace server
{

Frame::Frame()
        : _compound( 0 )
        , _buffers( eq::Frame::BUFFER_UNDEFINED )
        , _type( eq::Frame::TYPE_MEMORY )
        , _masterFrameData( 0 )
{
    _data.zoom.invalidate(); // to inherit zoom from compound unless set
    for( unsigned i = 0; i < NUM_EYES; ++i )
        _frameData[i] = 0;
}

Frame::Frame( const Frame& from )
        : co::Object()
        , _compound( 0 )
        , _name( from._name )
        , _data( from._data )
        , _vp( from._vp )
        , _buffers( from._buffers )
        , _type( from._type )
        , _masterFrameData( 0 )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
        _frameData[i] = 0;
}

Frame::~Frame()
{
    EQASSERT( _datas.empty());
}

void Frame::getInstanceData( co::DataOStream& os )
{
    os << _inherit;
}

void Frame::applyInstanceData( co::DataIStream& is )
{
    EQUNREACHABLE;
    is >> _inherit;
}

void Frame::flush()
{
    unsetData();

    while( !_datas.empty( ))
    {
        FrameData* data = _datas.front();
        getLocalNode()->deregisterObject( data );
        _datas.pop_front();
    }

}

void Frame::unsetData()
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _frameData[i] = 0;
        _inputFrames[i].clear();
    }
}

void Frame::commitData()
{
    if( !_masterFrameData ) // not used
        return;

    for( unsigned i = 0; i< NUM_EYES; ++i )
    {
        if( _frameData[i] )
        {
            if( _frameData[i] != _masterFrameData )
                _frameData[i]->_data = _masterFrameData->_data;

            _frameData[i]->commit();
        }
    }
}

uint32_t Frame::commitNB()
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
        _inherit.frameData[i] = _frameData[i];

    return co::Object::commitNB();
}

void Frame::cycleData( const uint32_t frameNumber, const uint32_t eyes )
{
    _masterFrameData = 0;
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _inputFrames[i].clear();

        if( !(eyes & (1<<i))) // eye pass not used
        {
            _frameData[i] = 0;
            continue;
        }

        // reuse unused frame data
        FrameData*     data    = _datas.empty() ? 0 : _datas.back();
        const uint32_t latency = getAutoObsolete();
        const uint32_t dataAge = data ? data->getFrameNumber() : 0;

        if( data && dataAge < frameNumber-latency && frameNumber > latency )
            // not used anymore
            _datas.pop_back();
        else // still used - allocate new data
        {
            data = new FrameData;

            getLocalNode()->registerObject( data );
            data->setAutoObsolete( 1 ); // current + in use by render nodes
        }

        data->setFrameNumber( frameNumber );
    
        _datas.push_front( data );
        _frameData[i] = data;
        if( !_masterFrameData )
            _masterFrameData = data;
    }
}

void Frame::addInputFrame( Frame* frame, const uint32_t eyes )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        if( !(eyes & (1<<i)) ||  // eye pass not used
            !_frameData[i] )     // no output frame for eye pass
        {
            frame->_frameData[i] = 0;
        }
        else
        {
            frame->_frameData[i] = _frameData[i];
            _inputFrames[i].push_back( frame );
        }
    }
}

co::ObjectVersion Frame::getDataVersion( const Eye eye ) const
{
    return co::ObjectVersion( _frameData[ co::base::getIndexOfLastBit( eye ) ] );
}

std::ostream& operator << ( std::ostream& os, const Frame* frame )
{
    if( !frame )
        return os;
    
    os << disableFlush << "frame" << endl;
    os << "{" << endl << indent;
      
    const std::string& name = frame->getName();
    os << "name     \"" << name << "\"" << endl;

    const uint32_t buffers = frame->getBuffers();
    if( buffers != eq::Frame::BUFFER_UNDEFINED )
    {
        os << "buffers  [";
        if( buffers & eq::Frame::BUFFER_COLOR )  os << " COLOR";
        if( buffers & eq::Frame::BUFFER_DEPTH )  os << " DEPTH";
        os << " ]" << endl;
    }

    const eq::Frame::Type frameType = frame->getType();
    if( frameType != eq::Frame::TYPE_MEMORY )
        os  << frameType;
    
    const eq::Viewport& vp = frame->getViewport();
    if( vp != eq::Viewport::FULL )
        os << "viewport " << vp << endl;

    const eq::Zoom& zoom = frame->getZoom();
    if( zoom.isValid() && zoom != eq::Zoom::NONE )
        os << zoom << endl;

    os << exdent << "}" << endl << enableFlush;
    return os;
}

}
}
