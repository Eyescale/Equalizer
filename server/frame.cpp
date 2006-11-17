
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "compound.h"
#include "frameBuffer.h"

#include <eq/client/object.h>
#include <eq/net/session.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

Frame::Frame()
        : eqNet::Object( eq::Object::TYPE_FRAME ),
          _compound( NULL ),
          _buffer( NULL )
{
    _data.format = eq::Frame::FORMAT_UNDEFINED;
    setInstanceData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::Frame( const Frame& from )
        : eqNet::Object( eq::Object::TYPE_FRAME ),
          _compound( NULL ),
          _buffer( NULL )
{
    _data = from._data;
    _name = from._name;
    _vp   = from._vp;
    setInstanceData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::~Frame()
{
    EQASSERT( _buffers.empty());
}

void Frame::flush()
{
    eqNet::Session* session = getSession();
    EQASSERT( session );
    while( !_buffers.empty( ))
    {
        FrameBuffer* buffer = _buffers.front();
        session->deregisterObject( buffer );
        _buffers.pop_front();
    }
    _buffer = NULL;
    _inputFrames.clear();
}

void Frame::updateInheritData( const Compound* compound )
{
    _inherit = _data;
    if( _inherit.format == eq::Frame::FORMAT_UNDEFINED )
        _inherit.format = compound->getInheritFormat();

    if( _buffer )
    {
        _inherit.buffer.objectID = _buffer->getID();
        _inherit.buffer.version  = _buffer->getVersion();
    }
    else
        _inherit.buffer.objectID = EQ_ID_INVALID;
}

void Frame::cycleBuffer( const uint32_t frameNumber )
{
    // find unused frame buffer
    FrameBuffer*   buffer    = _buffers.empty() ? NULL : _buffers.back();
    const uint32_t latency   = getAutoObsoleteCount();
    const uint32_t bufferAge = buffer ? buffer->getFrameNumber() : 0;

    if( bufferAge < frameNumber-latency && frameNumber > latency )
        // not used anymore
        _buffers.pop_back();
    else
    {
        buffer = new FrameBuffer;
        
        eqNet::Session* session = getSession();
        EQASSERT( session );

        session->registerObject( buffer, session->getLocalNode( ));
        buffer->setAutoObsolete( 1 ); // current + in use by render nodes
    }

    buffer->setFrameNumber( frameNumber );
    
    _buffers.push_front( buffer );
    _buffer = buffer;
    _inputFrames.clear();
}

void Frame::addInputFrame( Frame* frame )
{
    EQASSERT( _buffer );
    frame->_buffer = _buffer;
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

    os << exdent << "}" << endl << enableFlush;
    return os;
}
