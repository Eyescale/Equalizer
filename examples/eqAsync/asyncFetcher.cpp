
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
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

#define EQUALIZER_STATIC
#include <eq/util/objectManager.h>
#include <eq/util/objectManager.ipp>
#include <eq/util/bitmapFont.ipp>
#undef EQUALIZER_STATIC

#include "asyncFetcher.h"

#include "eqAsync.h"

#include <eq/eq.h>
#include <eq/client/system.h>
#ifdef AGL
#  include "aglWindowShared.h"
#endif
#ifdef GLX
#  include "glXWindowShared.h"
#endif

#include <ctime>

namespace eqAsync
{

static eq::SystemWindow* initSharedContextWindow( eq::Window* wnd )
{
    EQASSERT( wnd );

    // store old drawable of window and set window's drawable to FBO,
    // create another (shared) osWindow and restore original drowable
    const int32_t drawable =
        wnd->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    const int32_t stencil =
        wnd->getIAttribute( eq::Window::IATTR_PLANES_STENCIL );
    wnd->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, eq::OFF );

    const eq::Pipe* pipe = wnd->getPipe();
    EQASSERT( pipe );

    eq::SystemWindow* sharedContextWindow = 0;

    const std::string& ws = pipe->getWindowSystem().getName();

#ifdef GLX
    if( ws == "GLX" )
    {
        EQINFO << "Using GLXWindow" << std::endl;
        sharedContextWindow = new GLXWindowShared( wnd );
    }
#endif
#ifdef AGL
    if( ws == "AGL" )
    {
        EQINFO << "Using AGLWindow" << std::endl;
        sharedContextWindow = new AGLWindowShared( wnd );
    }
#endif
#ifdef WGL
    if( ws == "WGL" )
    {
        EQINFO << "Using WGLWindow" << std::endl;
        sharedContextWindow = new eq::WGLWindow( wnd );
    }
#endif
    if( !sharedContextWindow )
    {
        EQERROR << "Window system " << pipe->getWindowSystem()
                << " not implemented or supported" << std::endl;
        return 0;
    }

    if( !sharedContextWindow->configInit( ))
    {
        EQWARN << "OS Window initialization failed: " << std::endl;
        delete sharedContextWindow;
        return 0;
    }

    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
    wnd->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, stencil );

    sharedContextWindow->makeCurrent();

    EQWARN << "Async fetcher initialization finished" << std::endl;
    return sharedContextWindow;
}


static void deleteSharedContextWindow( eq::Window* wnd,
                                       eq::SystemWindow** sharedContextWindow,
                                   AsyncFetcher::ObjectManager** objectManager )
{
    EQWARN << "Deleting shared context" << std::endl;
    if( !sharedContextWindow || !*sharedContextWindow )
        return;

    if( *objectManager )
    {
        (*objectManager)->deleteAll();
        delete *objectManager;
        *objectManager = 0;
    }

    const int32_t drawable =
        wnd->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::FBO );

    (*sharedContextWindow)->configExit(); // mb set window to 0 before that?

    delete *sharedContextWindow;
    *sharedContextWindow = 0;

    wnd->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
}


AsyncFetcher::AsyncFetcher()
    : co::base::Thread()
    , _wnd( 0 )
    , _objectManager( 0 )
    , _sharedContextWindow( 0 )
{
    _tmpTexture = new GLbyte[ 64*64*4 ];
}


AsyncFetcher::~AsyncFetcher()
{
    if( _wnd && _sharedContextWindow )
        deleteSharedContextWindow( _wnd, &_sharedContextWindow,
                                   &_objectManager );

    delete [] _tmpTexture;
}


const GLEWContext* AsyncFetcher::glewGetContext() const
{
    return _sharedContextWindow->glewGetContext();
}


/**
 *  Function for creating and holding of shared context.
 *  Generation and uploading of new textures over some period with sleep time.
 */
void AsyncFetcher::run()
{
    EQASSERT( !_sharedContextWindow );
    _sharedContextWindow = initSharedContextWindow( _wnd );
    _outQueue.push( TextureId( )); // unlock pipe thread
    if( !_sharedContextWindow )
        return;

    _objectManager = new ObjectManager( glewGetContext( ));
    EQINFO << "async fetcher initialized: " << _wnd << std::endl;

    int i = 0;
    bool running = true;
    co::base::sleep( 1000 ); // imitate loading of the first texture
    while( running )
    {
        // generate new texture
        eq::util::Texture* tx = _objectManager->newEqTexture( ++i,
                                                              GL_TEXTURE_2D );
        tx->init( GL_RGBA8, 64, 64 );

        int j = 0;
        co::base::RNG rng;
        for( int y = 0; y < 64; ++y )
        {
            for( int x = 0; x < 64; ++x )
            {
                const GLbyte rnd = rng.get< uint8_t >() % 127;
                const GLbyte val = (x / 8) % 2 == (y / 8) % 2 ? rnd : 0;
                _tmpTexture[ j++ ] = val;
                _tmpTexture[ j++ ] = val;
                _tmpTexture[ j++ ] = val;
                _tmpTexture[ j++ ] = val;
            }
        }
        tx->upload( 64, 64, _tmpTexture );
        EQ_GL_CALL( glFinish( ));

        // add new texture to the pool
        _outQueue.push( TextureId( tx->getName( ), i ));

        // imitate hard work of loading something else
        co::base::sleep( rng.get< uint32_t >() % 5000u );

        // clean unused textures
        int keyToDelete = 0;
        while( _inQueue.tryPop( keyToDelete ))
        {
            if( keyToDelete )
            {
                EQWARN << "Deleting eq texture " << keyToDelete << std::endl;
                _objectManager->deleteEqTexture( keyToDelete );
            }
            else
                running = false;
        }
    }
    deleteSharedContextWindow( _wnd, &_sharedContextWindow, &_objectManager );
}

} //namespace eqAsync
