
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

#include <eq/client/gl.h>

namespace eq
{
namespace util
{
namespace detail
{
class Accum
{
public:
    Accum( const GLEWContext* const gl )
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
    : impl_( new detail::Accum( glewContext ))
{
}

Accum::~Accum()
{
    exit();
    delete impl_;
}

bool Accum::init( const PixelViewport& pvp, GLuint textureFormat )
{
    if( usesFBO( ))
    {
        impl_->abo = new AccumBufferObject( impl_->glewContext );
        if( !impl_->abo->init( pvp, textureFormat ))
        {
            delete impl_->abo;
            impl_->abo = 0;
            return false;
        }
    }

    if( impl_->totalSteps == 0 )
        impl_->totalSteps = getMaxSteps();

    impl_->pvp = pvp;
    return ( impl_->totalSteps > 0 );
}

void Accum::exit()
{
    clear();

    if( impl_->abo )
        impl_->abo->exit();

    delete impl_->abo;
    impl_->abo = 0;
}

void Accum::clear()
{
    impl_->numSteps = 0;
}

bool Accum::resize( const int width, const int height )
{
    return resize( PixelViewport( 0, 0, width, height ) );
}

bool Accum::resize( const PixelViewport& pvp )
{
    if( impl_->pvp == pvp )
        return false;

    impl_->pvp = pvp;
    if( usesFBO( ))
        return impl_->abo->resize( pvp );
    return true;
}

void Accum::accum()
{
    LBASSERT( impl_->numSteps < impl_->totalSteps );

    if( impl_->abo )
    {
        if( impl_->numSteps == 0 )
            impl_->abo->load( 1.0f );
        else
            impl_->abo->accum( 1.0f );
    }
    else
    {
        // This is the only working implementation on MacOS found at the moment.
        // glAccum function seems to be implemented differently.
        if( impl_->numSteps == 0 )
#ifdef Darwin
            glAccum( GL_LOAD, 1.0f / impl_->totalSteps );
#else
            glAccum( GL_LOAD, 1.0f );
#endif
        else
#ifdef Darwin
            glAccum( GL_ACCUM, 1.0f / impl_->totalSteps );
#else
            glAccum( GL_ACCUM, 1.0f );
#endif
    }

    ++impl_->numSteps;
}

void Accum::display()
{
    LBASSERT( impl_->numSteps <= impl_->totalSteps );

    if( impl_->abo )
        impl_->abo->display( 1.0f / impl_->numSteps );
    else
    {
#ifdef Darwin
        const float factor = float( impl_->totalSteps ) / impl_->numSteps;
#else
        const float factor = 1.0f / impl_->numSteps;
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
    return impl_->numSteps;
}

void Accum::setTotalSteps( uint32_t totalSteps )
{
    impl_->totalSteps = totalSteps;
}

uint32_t Accum::getTotalSteps()
{
    return impl_->totalSteps;
}

bool Accum::usesFBO() const
{
    return usesFBO( glewGetContext( ));
}


const GLEWContext* Accum::glewGetContext() const
{
    return impl_->glewContext;
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

