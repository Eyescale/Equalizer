
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

namespace eq
{
namespace util
{

Accum::Accum( const GLEWContext* const glewContext )
    : _glewContext( glewContext )
    , _width( 0 )
    , _height( 0 )
    , _abo( 0 )
    , _numSteps( 0 )
    , _totalSteps( 0 )
{
    EQASSERT( glewContext );
}

Accum::~Accum()
{
    exit();
}

bool Accum::init( const PixelViewport& pvp, GLuint textureFormat )
{
    if( usesFBO( ))
    {
        _abo = new AccumBufferObject( _glewContext );
        if( !_abo->init( pvp, textureFormat ))
        {
            delete _abo;
            _abo = 0;
            return false;
        }
    }

    if( _totalSteps == 0 )
        _totalSteps = getMaxSteps();

    _width = pvp.w;
    _height = pvp.h;

    return ( _totalSteps > 0 );
}

void Accum::exit()
{
    clear();

    if( _abo )
        _abo->exit();

    delete _abo;
    _abo = 0;
}

void Accum::clear()
{
    _numSteps = 0;
}

bool Accum::resize( const int width, const int height )
{
    if( usesFBO( ))
    {
        if( _abo->getWidth() == width && _abo->getHeight() == height )
            return false;

        return _abo->resize( width, height );
    }
    
    if( width != _width || height != _height )
    {
        _width = width;
        _height = height;
        return true;
    }

    return false;
}

void Accum::accum()
{
    EQASSERT( _numSteps < _totalSteps );

    if( _abo )
    {
        if( _numSteps == 0 )
            _abo->load( 1.0f );
        else
            _abo->accum( 1.0f );
    }
    else
    {
        // This is the only working implementation on MacOS found at the moment.
        // glAccum function seems to be implemented differently.
        if( _numSteps == 0 )
#ifdef Darwin
            glAccum( GL_LOAD, 1.0f / _totalSteps );
#else
            glAccum( GL_LOAD, 1.0f );
#endif
        else
#ifdef Darwin
            glAccum( GL_ACCUM, 1.0f / _totalSteps );
#else
            glAccum( GL_ACCUM, 1.0f );
#endif
    }

    ++_numSteps;
}

void Accum::display()
{
    EQASSERT( _numSteps <= _totalSteps );

    if( _abo )
    {
        const float factor = 1.0f / _numSteps;
        _abo->display( factor );
    }
    else
    {
#ifdef Darwin
        const float factor = static_cast<float>( _totalSteps ) / _numSteps;
#else
        const float factor = 1.0f / _numSteps;
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

bool Accum::usesFBO() const
{
    return usesFBO( glewGetContext( ));
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

