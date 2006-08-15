
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameBuffer.h"

#include <eq/client/object.h>
#include <eq/net/session.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

Frame::Frame()
        : eqNet::Object( eq::Object::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM ),
          _buffer( NULL )
{
    setDistributedData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::Frame( const Frame& from )
        : eqNet::Object( eq::Object::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM ),
          _buffer( NULL )
{
    setDistributedData( &_inherit, sizeof( eq::Frame::Data ));
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
}

void Frame::updateInheritData( const Compound* compound )
{
}

void Frame::cycleFrameBuffer( const uint32_t frameNumber, const uint32_t maxAge)
{

    // find unused frame buffer
    FrameBuffer* buffer = _buffers.empty() ? NULL : _buffers.back();
    
    if( buffer && 
        buffer->getFrameNumber() < frameNumber-maxAge ) // not used anymore

        _buffers.pop_back();
    else
    {
        buffer = new FrameBuffer;
        
        eqNet::Session* session = getSession();
        EQASSERT( session );

        session->registerObject( buffer, session->getLocalNode( ));
    }

    buffer->setFrameNumber( frameNumber );
    buffer->commit();
    
    _buffers.push_front( buffer );
    _setFrameBuffer( buffer );
}

void Frame::setOutputFrame( Frame* frame )
{
    EQASSERT( frame->_buffer );
    _setFrameBuffer( frame->_buffer );
}

void Frame::_setFrameBuffer( FrameBuffer* buffer )
{
    _buffer = buffer;
    if( !buffer )
        return;

    _inherit.buffer.objectID = buffer->getID();
    _inherit.buffer.version  = buffer->getVersion();
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
