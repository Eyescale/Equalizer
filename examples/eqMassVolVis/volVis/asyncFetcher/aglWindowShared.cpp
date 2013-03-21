
/* Copyright (c)  2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                2007-2011, Maxim Makhinya <maxmah@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "aglWindowShared.h"

#ifdef AGL

namespace massVolVis
{

AGLWindowShared::AGLWindowShared( eq::Window* parent,
                                  CGDirectDisplayID displayID )
    : eq::AGLWindow( parent, displayID )
{}


AGLPixelFormat AGLWindowShared::chooseAGLPixelFormat()
{
    eq::Global::enterCarbon();

    CGOpenGLDisplayMask glDisplayMask =
        CGDisplayIDToOpenGLDisplayMask( getCGDisplayID( ));

    // build attribute list
    std::vector<GLint> attributes;

    attributes.push_back( AGL_RGBA );
    attributes.push_back( GL_TRUE );
    attributes.push_back( AGL_ACCELERATED );
    attributes.push_back( GL_TRUE );

    //
    //  This condition is the only difference from the original function
    //
    if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == eq::ON )
    {
        attributes.push_back( AGL_FULLSCREEN );
    }

    attributes.push_back( AGL_DISPLAY_MASK );
    attributes.push_back( glDisplayMask );

    GLint colorSize = getIAttribute( eq::Window::IATTR_PLANES_COLOR );
    if( colorSize != eq::OFF )
    {
        switch( colorSize )
        {
          case eq::RGBA16F:
            attributes.push_back( AGL_COLOR_FLOAT );
            colorSize = 16;
            break;
          case eq::RGBA32F:
            attributes.push_back( AGL_COLOR_FLOAT );
            colorSize = 32;
            break;
          case eq::AUTO:
            colorSize = 8;
            break;
          default:
              break;
        }

        attributes.push_back( AGL_RED_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( AGL_GREEN_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( AGL_BLUE_SIZE );
        attributes.push_back( colorSize );

        const int alphaSize = getIAttribute( eq::Window::IATTR_PLANES_ALPHA );
        if( alphaSize > 0 || alphaSize == eq::AUTO )
        {
            attributes.push_back( AGL_ALPHA_SIZE );
            attributes.push_back( alphaSize > 0 ? alphaSize : colorSize );
        }
    }

    const int depthSize = getIAttribute( eq::Window::IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == eq::AUTO )
    {
        attributes.push_back( AGL_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }
    const int stencilSize = getIAttribute( eq::Window::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == eq::AUTO )
    {
        attributes.push_back( AGL_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }
    const int accumSize  = getIAttribute( eq::Window::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( eq::Window::IATTR_PLANES_ACCUM_ALPHA);
    if( accumSize >= 0 )
    {
        attributes.push_back( AGL_ACCUM_RED_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_GREEN_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_BLUE_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha >= 0 ? accumAlpha : accumSize );
    }
    else if( accumAlpha >= 0 )
    {
        attributes.push_back( AGL_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha );
    }

    const int samplesSize  = getIAttribute( eq::Window::IATTR_PLANES_SAMPLES );
    if( samplesSize >= 0 )
    {
        attributes.push_back( AGL_SAMPLE_BUFFERS_ARB );
        attributes.push_back( 1 );
        attributes.push_back( AGL_SAMPLES_ARB );
        attributes.push_back( samplesSize );
    }

    if( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == eq::ON ||
        ( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == eq::AUTO &&
          getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )     == eq::WINDOW ))
    {
        attributes.push_back( AGL_DOUBLEBUFFER );
        attributes.push_back( GL_TRUE );
    }
    if( getIAttribute( eq::Window::IATTR_HINT_STEREO ) == eq::ON )
    {
        attributes.push_back( AGL_STEREO );
        attributes.push_back( GL_TRUE );
    }

    attributes.push_back( AGL_NONE );

    // build backoff list, least important attribute last
    std::vector<int> backoffAttributes;
    if( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == eq::AUTO &&
        getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )     == eq::WINDOW  )

        backoffAttributes.push_back( AGL_DOUBLEBUFFER );

    if( stencilSize == eq::AUTO )
        backoffAttributes.push_back( AGL_STENCIL_SIZE );

    // choose pixel format
    AGLPixelFormat pixelFormat = 0;
    while( true )
    {
        pixelFormat = aglCreatePixelFormat( &attributes.front( ));

        if( pixelFormat ||              // found one or
            backoffAttributes.empty( )) // nothing else to try

            break;

        // Gradually remove backoff attributes
        const GLint attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<GLint>::iterator iter = find( attributes.begin(),
                                             attributes.end(), attribute );
        LBASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two item (attr, value)
    }

    if( !pixelFormat )
        setError( eq::ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );

    eq::Global::leaveCarbon();
    return pixelFormat;
}

} // namespace massVolVis

#endif // AGL
