
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "compound.h"
#include "frameData.h"

#include <eq/client/object.h>
#include <eq/net/session.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

Frame::Frame()
        : _compound( 0 ),
          _frameData( 0 )
{
    _data.buffers = eq::Frame::BUFFER_UNDEFINED;
    setInstanceData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::Frame( const Frame& from )
        : _compound( 0 ),
          _frameData( 0 )
{
    _data = from._data;
    _name = from._name;
    _vp   = from._vp;
    setInstanceData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::~Frame()
{
    EQASSERT( _datas.empty());
}

void Frame::flush()
{
    eqNet::Session* session = getSession();
    EQASSERT( session );
    while( !_datas.empty( ))
    {
        FrameData* data = _datas.front();
        session->deregisterObject( data );
        _datas.pop_front();
    }
    _frameData = 0;
    _inputFrames.clear();
}

void Frame::updateInheritData( const Compound* compound )
{
    _inherit = _data;
    if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
        _inherit.buffers = compound->getInheritBuffers();

    if( _frameData )
    {
        _inherit.frameData.objectID = _frameData->getID();
        _inherit.frameData.version  = _frameData->getVersion();
    }
    else
        _inherit.frameData.objectID = EQ_ID_INVALID;
}

void Frame::cycleData( const uint32_t frameNumber )
{
    // find unused frame data
    FrameData*     data    = _datas.empty() ? 0 : _datas.back();
    const uint32_t latency = getAutoObsoleteCount();
    const uint32_t dataAge = data ? data->getFrameNumber() : 0;

    if( data && dataAge < frameNumber-latency && frameNumber > latency )
        // not used anymore
        _datas.pop_back();
    else
    {
        data = new FrameData;
        
        eqNet::Session* session = getSession();
        EQASSERT( session );

        session->registerObject( data );
        data->setAutoObsolete( 1 ); // current + in use by render nodes
    }

    data->setFrameNumber( frameNumber );
    
    _datas.push_front( data );
    _frameData = data;
    _inputFrames.clear();
}

void Frame::addInputFrame( Frame* frame )
{
    EQASSERT( _frameData );
    frame->_frameData = _frameData;
    _inputFrames.push_back( frame );
}

std::ostream& eqs::operator << ( std::ostream& os, const Frame* frame )
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

    const eq::Viewport& vp = frame->getViewport();
    if( !vp.isFullScreen( ))
        os << "viewport " << vp << endl;

    os << exdent << "}" << endl << enableFlush;
    return os;
}
