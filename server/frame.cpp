
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "compound.h"
#include "frameData.h"

#include <eq/net/session.h>

using namespace eq::base;
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
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
        _frameData[i] = 0;
}

Frame::Frame( const Frame& from )
        : net::Object()
        , _compound( 0 )
        , _name( from._name )
        , _data( from._data )
        , _vp( from._vp )
        , _buffers( from._buffers )
        , _type( from._type )
        , _masterFrameData( 0 )
{
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
        _frameData[i] = 0;
}

Frame::~Frame()
{
    EQASSERT( _datas.empty());
}

void Frame::getInstanceData( net::DataOStream& os )
{
    os << _inherit;
}

void Frame::applyInstanceData( net::DataIStream& is )
{
    EQUNREACHABLE;
    is >> _inherit;
}

void Frame::flush()
{
    unsetData();

    net::Session* session = getSession();
    EQASSERT( session );
    while( !_datas.empty( ))
    {
        FrameData* data = _datas.front();
        session->deregisterObject( data );
        _datas.pop_front();
    }

}

void Frame::unsetData()
{
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
    {
        _frameData[i] = 0;
        _inputFrames[i].clear();
    }
}

void Frame::commitData()
{
    if( !_masterFrameData ) // not used
        return;

    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
    {
        if( _frameData[i] )
        {
            if( _frameData[i] != _masterFrameData )
                _frameData[i]->_data = _masterFrameData->_data;

            _frameData[i]->commit();
        }
    }
}

uint32_t Frame::commit()
{
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
    {
        if( _frameData[i] )
        {
            _inherit.frameData[i].id      = _frameData[i]->getID();
            _inherit.frameData[i].version = _frameData[i]->getVersion();
        }
        else
            _inherit.frameData[i].id = EQ_ID_INVALID;
    }

    return net::Object::commit();
}

void Frame::cycleData( const uint32_t frameNumber, const uint32_t eyes )
{
    _masterFrameData = 0;
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
    {
        _inputFrames[i].clear();

        if( !(eyes & (1<<i))) // eye pass not used
        {
            _frameData[i] = 0;
            continue;
        }

        // reuse unused frame data
        FrameData*     data    = _datas.empty() ? 0 : _datas.back();
        const uint32_t latency = getAutoObsoleteCount();
        const uint32_t dataAge = data ? data->getFrameNumber() : 0;

        if( data && dataAge < frameNumber-latency && frameNumber > latency )
            // not used anymore
            _datas.pop_back();
        else // still used - allocate new data
        {
            data = new FrameData;
        
            net::Session* session = getSession();
            EQASSERT( session );

            session->registerObject( data );
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
    for( unsigned i = 0; i<eq::EYE_ALL; ++i )
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

std::ostream& operator << ( std::ostream& os, const Frame* frame )
{
    if( !frame )
        return os;
    
    os << disableFlush << "Frame" << endl;
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
