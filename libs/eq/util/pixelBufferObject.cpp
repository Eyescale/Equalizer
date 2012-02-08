
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
 *               2012, Stefan Eilemann <eile@eyescale.ch>
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
#include <co/base/scopedMutex.h>

namespace eq
{
namespace util
{
namespace detail
{
class PixelBufferObject
{
public:
    PixelBufferObject( const GLEWContext* glewContext,
                       const bool threadSafe )
            : lock_( threadSafe ? new co::base::Lock : 0 )
            , pboID( 0 )
            , size( 0 )
            , _glewContext( glewContext )
            , _type( 0 )
        {}

    ~PixelBufferObject()
        {
            if( isInitialized( ))
                EQWARN << "PBO was not freed" << std::endl;

            delete lock_;
        }

    const GLEWContext* glewGetContext() const { return _glewContext; }

    bool isInitialized() const { return pboID != 0; }

    bool setup( const ssize_t newSize, const GLuint type )
        {
            if( !GLEW_ARB_pixel_buffer_object )
            {
                _setError( ERROR_PBO_UNSUPPORTED );
                return false;
            }

            if( newSize <= 0 )
            {
                _setError( ERROR_PBO_SIZE_TOO_SMALL );
                destroy();
                return false;
            }

            if( pboID == 0 )
            {
                EQ_GL_CALL( glGenBuffersARB( 1, &pboID ));
            }
            if( pboID == 0 )
            {
                _setError( ERROR_PBO_NOT_INITIALIZED );
                return false;
            }

            if( _type == type && size >= newSize )
                return true;

            _type = type;
            size = newSize;

            switch( type )
            {
              case GL_READ_ONLY_ARB:
                  bind();
                  EQ_GL_CALL( glBufferDataARB( GL_PIXEL_PACK_BUFFER_ARB, size,
                                               0, GL_STREAM_READ_ARB ));
                  unbind();
                  break;

              case GL_WRITE_ONLY_ARB:
                  bind();
                  EQ_GL_CALL( glBufferDataARB( GL_PIXEL_UNPACK_BUFFER_ARB, size,
                                               0, GL_STREAM_DRAW_ARB ));
                  unbind();
                  break;

              default:
                  _setError( ERROR_PBO_TYPE_UNSUPPORTED );
                  destroy();
                  return false;
            }
            return true;
        }

    void destroy()
        {
            if( pboID != 0 )
            {
                EQ_GL_CALL( glDeleteBuffersARB( 1, &pboID ));
            }
            pboID = 0;
            size = 0;
            _type = 0;
        }


    const void* mapRead() const
        {
            if( !_testInitialized() )
                return 0;

            if( _type != GL_READ_ONLY_ARB )
            {
                _setError( ERROR_PBO_WRITE_ONLY );
                return 0;
            }

            bind();
            return glMapBufferARB( GL_PIXEL_PACK_BUFFER_ARB, _type );
        }

    void* mapWrite()
        {
            if( !_testInitialized() )
                return 0;

            if( _type != GL_WRITE_ONLY_ARB )
            {
                _setError( ERROR_PBO_READ_ONLY );
                return 0;
            }

            bind();
            // cancel all draw operations on this buffer to prevent stalling
            EQ_GL_CALL( glBufferDataARB( GL_PIXEL_UNPACK_BUFFER_ARB, size, 0, 
                                         GL_STREAM_DRAW_ARB ));
            return glMapBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB, _type );
        }

    void unmap() const
        {
            _testInitialized();
            EQ_GL_CALL( glUnmapBufferARB( _getName( )));
            unbind();
        }

    bool bind() const
        {
            if( !_testInitialized() )
                return false;

            EQ_GL_CALL( glBindBufferARB( _getName(), pboID ));
            return true;
        }

    void unbind() const
        {
            _testInitialized();
            EQ_GL_CALL( glBindBufferARB( _getName(), 0 ));
        }

    void lock()   const { if( lock_ ) lock_->set();   }
    void unlock() const { if( lock_ ) lock_->unset(); }

    mutable co::base::Lock* lock_;
    mutable co::base::Error error;   //!< The reason for the last error
    GLuint  pboID; //!< the PBO GL name
    ssize_t size;  //!< size of the allocated PBO buffer

private:
    const GLEWContext* const _glewContext;
    GLuint _type; //!< GL_READ_ONLY_ARB or GL_WRITE_ONLY_ARB

    void _setError( const int32_t e ) const { error = co::base::Error( e ); }

    GLuint _getName() const
        {
            return _type == GL_READ_ONLY_ARB ?
                GL_PIXEL_PACK_BUFFER_ARB : GL_PIXEL_UNPACK_BUFFER_ARB;
        }

    bool _testInitialized() const
        {
            if( isInitialized( ))
                return true;

            _setError( ERROR_PBO_NOT_INITIALIZED );
            return false;
        }
};
}

PixelBufferObject::PixelBufferObject( const GLEWContext* glewContext,
                                      const bool threadSafe )
        : _impl( new detail::PixelBufferObject( glewContext, threadSafe ))
{
}

PixelBufferObject::~PixelBufferObject()
{
    delete _impl;
}

bool PixelBufferObject::setup( const ssize_t size, const GLuint type )
{
    co::base::ScopedWrite mutex( _impl->lock_ );
    return _impl->setup( size, type );
}

void PixelBufferObject::destroy()
{
    co::base::ScopedWrite mutex( _impl->lock_ );
    _impl->destroy();
}

const void* PixelBufferObject::mapRead() const
{
    _impl->lock();
    return _impl->mapRead();
}

void* PixelBufferObject::mapWrite()
{
    _impl->lock();
    return _impl->mapWrite();
}

void PixelBufferObject::unmap() const
{
    _impl->unmap();
    _impl->unlock();
}

bool PixelBufferObject::bind() const
{
    _impl->lock();
    return _impl->bind();
}

void PixelBufferObject::unbind() const
{
    _impl->unbind();
    _impl->unlock();
}

ssize_t PixelBufferObject::getSize() const
{
    return _impl->size;
}

const co::base::Error& PixelBufferObject::getError() const
{
    return _impl->error;
}

bool PixelBufferObject::isInitialized() const
{
    return _impl->isInitialized();
}

bool PixelBufferObject::isThreadSafe() const
{
    return _impl->lock_ != 0;
}

}
}
