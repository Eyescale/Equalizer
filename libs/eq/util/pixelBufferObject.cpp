
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
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

#include "pixelBufferObject.h"

#include <eq/client/error.h>
#include <co/base/debug.h>

namespace eq
{
namespace util
{

#define glewGetContext() glewContext


PixelBufferObject::PixelBufferObject( const GLEWContext* glewContext,
                                      const bool threadSafe )
    : _pboId( 0 )
    , _size( 0 )
    , _name( 0 )
    , _type( 0 )
    , _op( 0 )
    , _initialized( false )
    , _threadSafe( threadSafe )
    , _initCtx( glewContext )
{
}


PixelBufferObject::~PixelBufferObject()
{
    if( !isInitialized( ))
        return;

    EQWARN << "deleting PBO in destructor since it was not deleted manually"
           << std::endl;

    _destroy( _initCtx );
}


bool PixelBufferObject::_testIfInitialized() const
{
    if( isInitialized( ))
        return true;

    _setError( ERROR_PBO_NOT_INITIALIZED );
    return false;
}


bool PixelBufferObject::init( const int32_t newSize,
                              const GLEWContext* glewContext, const bool read )
{
    _lock();
    const bool result = _init( newSize, glewContext, read );
    _unlock();

    return result;
}
bool PixelBufferObject::_init( const int32_t newSize,
                               const GLEWContext* glewContext, const bool read )
{
    if( !GLEW_ARB_pixel_buffer_object )
    {
        _setError( ERROR_PBO_UNSUPPORTED );
        return false;
    }

    if( newSize <= 0 )
    {
        _setError( ERROR_PBO_SIZE_TOO_SMALL );
        _destroy( glewContext );
        return false;
    }

    if( !_pboId )
        EQ_GL_CALL( glGenBuffersARB( 1, &_pboId ));

    if( _pboId == 0 )
    {
        _setError( ERROR_PBO_NOT_INITIALIZED );
        _initialized = false;
        return false;
    }
    _initialized = true;

    // check if new pbo is of the same type as before;
    // if new one is also smaller in size then don't do anything
    const GLuint newName = 
                   read ? GL_PIXEL_PACK_BUFFER_ARB : GL_PIXEL_UNPACK_BUFFER_ARB;
    const GLuint newType = read ? GL_READ_ONLY_ARB  : GL_WRITE_ONLY_ARB;
    const GLuint newOp   = read ? GL_STREAM_READ_ARB: GL_STREAM_DRAW_ARB;

    if( _name == newName && _size >= newSize && _initialized )
        return true;

    _name = newName;
    _type = newType;
    _size = newSize;
    _op   = newOp;

    _bind( glewContext );
    EQ_GL_CALL( glBufferDataARB( _name, _size, 0, _op ));
    _unbind( glewContext );

    return true;
}

const void* PixelBufferObject::mapRead( const GLEWContext* glewContext ) const
{
    _lock();

    if( !_testIfInitialized() )
        return 0;

    if( _type != GL_READ_ONLY_ARB )
    {
        _setError( ERROR_PBO_WRITE_ONLY );
        return 0;
    }

    _bind( glewContext );
    return glMapBufferARB( _name, _type );
}


void* PixelBufferObject::mapWrite( const GLEWContext* glewContext )
{
    EQWARN << "void* PixelBufferObject::map( const GLEWContext* glewContext )" << std::endl;
    _lock();

    if( !_testIfInitialized() )
        return 0;

    if( _type != GL_WRITE_ONLY_ARB )
    {
        _setError( ERROR_PBO_READ_ONLY );
        return 0;
    }

    _bind( glewContext );
    // cancel all other draw operations on this buffer to prevent stalling
    EQ_GL_CALL( glBufferDataARB( _name, _size, 0, _op ));
    return glMapBufferARB( _name, _type );
}


bool PixelBufferObject::bind( const GLEWContext* glewContext ) const
{
    _lock();
    return _bind( glewContext );
}
bool PixelBufferObject::_bind(    const GLEWContext* glewContext ) const
{
    if( !_testIfInitialized() )
        return false;

    EQ_GL_CALL( glBindBufferARB( _name, _pboId ));
    return true;
}


void PixelBufferObject::unmap( const GLEWContext* glewContext ) const
{
    _unmap( glewContext );
    _unlock();
}
void PixelBufferObject::_unmap(   const GLEWContext* glewContext ) const
{
    _testIfInitialized();

    EQ_GL_CALL( glUnmapBufferARB( _name ));
    _unbind( glewContext );
}


void PixelBufferObject::unbind( const GLEWContext* glewContext ) const
{
    _unbind( glewContext );
    _unlock();
}
void PixelBufferObject::_unbind(  const GLEWContext* glewContext ) const
{
    _testIfInitialized();

    EQ_GL_CALL( glBindBufferARB( _name, 0 ));
}


void PixelBufferObject::destroy( const GLEWContext* glewContext )
{
    _lock();
    _destroy( glewContext );
    _unlock();
}
void PixelBufferObject::_destroy( const GLEWContext* glewContext )
{
    _size = 0;
    _name = 0;
    _type = 0;

    if( _pboId == 0 )
    {
        _initialized = false;
        return;
    }

    EQ_GL_CALL( glDeleteBuffersARB( 1, &_pboId ));
    _pboId = 0;
    _initialized = false;
}

void PixelBufferObject::_setError( const int32_t error ) const
{
    _error = co::base::Error( error );
}

}
}
