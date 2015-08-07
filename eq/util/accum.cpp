
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#include "accumBufferObject.h"
#include "accum.h"

#include <eq/gl.h>

namespace eq
{
namespace util
{
namespace detail
{
class Accum
{
public:
    explicit Accum( const GLEWContext* const gl )
        : glewContext( gl )
        , abo( 0 )
        , numSteps( 0 )
        , totalSteps( 0 )
    {
        LBASSERT( glewContext );
    }

    const GLEWContext* const glewContext;
    PixelViewport pvp;

    AccumBufferObject* abo;
    uint32_t numSteps;
    uint32_t totalSteps;
};
}

Accum::Accum( const GLEWContext* const glewContext )
    : _impl( new detail::Accum( glewContext ))
{
}

Accum::~Accum()
{
    this->exit();
    delete _impl;
}

bool Accum::init( const PixelViewport& pvp, GLuint textureFormat )
{
    if( usesFBO( ))
    {
        _impl->abo = new AccumBufferObject( _impl->glewContext );
        if( !_impl->abo->init( pvp, textureFormat ))
        {
            delete _impl->abo;
            _impl->abo = 0;
            return false;
        }
    }

    if( _impl->totalSteps == 0 )
        _impl->totalSteps = getMaxSteps();

    _impl->pvp = pvp;
    return ( _impl->totalSteps > 0 );
}

void Accum::exit()
{
    clear();

    if( _impl->abo )
        _impl->abo->exit();

    delete _impl->abo;
    _impl->abo = 0;
}

void Accum::clear()
{
    _impl->numSteps = 0;
}

bool Accum::resize( const int width, const int height )
{
    return resize( PixelViewport( 0, 0, width, height ) );
}

bool Accum::resize( const PixelViewport& pvp )
{
    if( _impl->pvp == pvp )
        return false;

    _impl->pvp = pvp;
    if( usesFBO( ))
        return _impl->abo->resize( pvp );
    return true;
}

void Accum::accum()
{
    LBASSERT( _impl->numSteps < _impl->totalSteps );

    if( _impl->abo )
    {
        if( _impl->numSteps == 0 )
            _impl->abo->load( 1.0f );
        else
            _impl->abo->accum( 1.0f );
    }
    else
    {
        // This is the only working implementation on MacOS found at the moment.
        // glAccum function seems to be implemented differently.
        if( _impl->numSteps == 0 )
#ifdef Darwin
            glAccum( GL_LOAD, 1.0f / _impl->totalSteps );
#else
            glAccum( GL_LOAD, 1.0f );
#endif
        else
#ifdef Darwin
            glAccum( GL_ACCUM, 1.0f / _impl->totalSteps );
#else
            glAccum( GL_ACCUM, 1.0f );
#endif
    }

    ++_impl->numSteps;
}

void Accum::display()
{
    LBASSERT( _impl->numSteps <= _impl->totalSteps );

    if( _impl->abo )
        _impl->abo->display( 1.0f / _impl->numSteps );
    else
    {
#ifdef Darwin
        const float factor = float( _impl->totalSteps ) / _impl->numSteps;
#else
        const float factor = 1.0f / _impl->numSteps;
#endif
        glAccum( GL_RETURN, factor );
    }
}

uint32_t Accum::getMaxSteps() const
{
    if( usesFBO( ))
        return 256;

    GLint accumBits;
    glGetIntegerv( GL_ACCUM_RED_BITS, &accumBits );
    return accumBits >= 16 ? 256 : 0;
}

uint32_t Accum::getNumSteps() const
{
    return _impl->numSteps;
}

void Accum::setTotalSteps( uint32_t totalSteps )
{
    _impl->totalSteps = totalSteps;
}

uint32_t Accum::getTotalSteps()
{
    return _impl->totalSteps;
}

bool Accum::usesFBO() const
{
    return usesFBO( glewGetContext( ));
}


const GLEWContext* Accum::glewGetContext() const
{
    return _impl->glewContext;
}

#define glewGetContext() glewContext

#ifdef Darwin
bool Accum::usesFBO( const GLEWContext* )
{
    return false;
}
#else
bool Accum::usesFBO( const GLEWContext* glewContext )
{
    return ( GLEW_EXT_framebuffer_object &&
           ( GLEW_VERSION_3_0 || GLEW_ARB_texture_float ));
}
#endif

#undef glewGetContext

}
}
