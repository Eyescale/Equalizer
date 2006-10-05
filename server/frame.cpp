
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
        : eqNet::Object( eq::Object::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM ),
          _buffer( NULL )
{
    _data.format = eq::Frame::FORMAT_UNDEFINED;
    setDistributedData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::Frame( const Frame& from )
        : eqNet::Object( eq::Object::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM ),
          _buffer( NULL )
{
    _data = from._data;
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
    _inherit = _data;
    if( _inherit.format == eq::Frame::FORMAT_UNDEFINED )
        _inherit.format = compound->getInheritFormat();
    if( _buffer )
        _buffer->setFormat( _inherit.format );
}

void Frame::cycleBuffer( const uint32_t frameNumber, const uint32_t maxAge )
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
    
    _buffers.push_front( buffer );
    _buffer = buffer;
}

void Frame::setOutputFrame( Frame* frame )
{
    EQASSERT( frame->_buffer );
    _buffer = frame->_buffer;
}

const void* Frame::pack( uint64_t* size )
{
    if( _buffer )
    {
        _buffer->commit();
        _inherit.buffer.objectID = _buffer->getID();
        _inherit.buffer.version  = _buffer->getVersion();
    }
    else
        _inherit.buffer.objectID = EQ_ID_INVALID;

    return Object::pack( size );
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
