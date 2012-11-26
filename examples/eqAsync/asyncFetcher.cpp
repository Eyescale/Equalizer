
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *                    2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "asyncFetcher.h"
#include "eqAsync.h"

#include <eq/eq.h>
#include <eq/client/system.h>

namespace eqAsync
{

static eq::SystemWindow* initSharedContextWindow( eq::Window* window )
{
    LBASSERT( window );

    // store old drawable of window and set window's drawable to OFF,
    // create another (shared) osWindow and restore original drawable
    const int32_t drawable =
        window->getIAttribute( eq::Window::IATTR_HINT_DRAWABLE );
    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, eq::OFF );

    const eq::Pipe* pipe = window->getPipe();
    LBASSERT( pipe );

    eq::SystemWindow* sharedWindow =
        pipe->getWindowSystem().createWindow( window );

    if( sharedWindow )
    {
        if( !sharedWindow->configInit( ))
        {
            LBWARN << "OS Window initialization failed: " << std::endl;
            delete sharedWindow;
            sharedWindow = 0;
        }
    }
    else
    {
        LBERROR << "Failed to create shared context window for "
                << pipe->getWindowSystem() << std::endl;
    }

    window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, drawable );
    window->makeCurrent();

    LBINFO << "Async fetcher initialization finished" << std::endl;
    return sharedWindow;
}


static void deleteSharedContextWindow( eq::Window* window,
                                       eq::SystemWindow** sharedWindow,
                                       eq::ObjectManager** objectManager )
{
    LBWARN << "Deleting shared context" << std::endl;
    if( !sharedWindow || !*sharedWindow )
        return;

    if( *objectManager )
    {
        (*objectManager)->deleteAll();
        delete *objectManager;
        *objectManager = 0;
    }

    (*sharedWindow)->configExit(); // mb set window to 0 before that?

    delete *sharedWindow;
    *sharedWindow = 0;
}


AsyncFetcher::AsyncFetcher()
    : lunchbox::Thread()
    , _window( 0 )
    , _objectManager( 0 )
    , _sharedWindow( 0 )
{
    _tmpTexture = new GLbyte[ 64*64*4 ];
}


AsyncFetcher::~AsyncFetcher()
{
    if( _window && _sharedWindow )
        deleteSharedContextWindow( _window, &_sharedWindow, &_objectManager );

    delete [] _tmpTexture;
}


const GLEWContext* AsyncFetcher::glewGetContext() const
{
    return _sharedWindow->glewGetContext();
}

void AsyncFetcher::setup( Window* window )
{
    _window = window;
    _sharedWindow = initSharedContextWindow( _window );    
}


/**
 *  Function for creating and holding of shared context.
 *  Generation and uploading of new textures over some period with sleep time.
 */
void AsyncFetcher::run()
{
    LBASSERT( _sharedWindow );
    if( !_sharedWindow )
        return;
        
    _sharedWindow->makeCurrent();
    _outQueue.push( TextureId( )); // unlock pipe thread    
      
    _objectManager = new eq::ObjectManager( glewGetContext( ));
    LBINFO << "async fetcher initialized: " << _window << std::endl;

    uint8_t* i = 0;
    bool running = true;
    lunchbox::sleep( 1000 ); // imitate loading of the first texture
    while( running )
    {
        // generate new texture
        eq::util::Texture* tx = _objectManager->newEqTexture( ++i,
                                                              GL_TEXTURE_2D );
        tx->init( GL_RGBA8, 64, 64 );

        int j = 0;
        lunchbox::RNG rng;
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
        _outQueue.push( TextureId( tx->getName(), i ));

        // imitate hard work of loading something else
        lunchbox::sleep( rng.get< uint32_t >() % 5000u );

        // clean unused textures
        const void* keyToDelete = 0;
        while( _inQueue.tryPop( keyToDelete ))
        {
            if( keyToDelete )
            {
                LBWARN << "Deleting eq texture " << keyToDelete << std::endl;
                _objectManager->deleteEqTexture( keyToDelete );
            }
            else
                running = false;
        }
    }
    deleteSharedContextWindow( _window, &_sharedWindow, &_objectManager );
}

} //namespace eqAsync
