
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012-2013, Stefan Eilemann <eile@eyescale.ch>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/system.h>

namespace eqAsync
{
AsyncFetcher::AsyncFetcher()
    : lunchbox::Thread()
    , _sharedWindow( 0 )
{
}

AsyncFetcher::~AsyncFetcher()
{
    stop();
}

const GLEWContext* AsyncFetcher::glewGetContext() const
{
    return _sharedWindow->glewGetContext();
}

static eq::SystemWindow* initSharedContextWindow( eq::Window* window )
{
    LBASSERT( window );

    eq::WindowSettings settings = window->getSettings();
    settings.setIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE, eq::OFF );
    const eq::Pipe* pipe = window->getPipe();
    eq::SystemWindow* sharedWindow =
        pipe->getWindowSystem().createWindow( window, settings );

    if( sharedWindow )
    {
        if( sharedWindow->configInit( ))
            sharedWindow->makeCurrent();
        else
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

    window->makeCurrent();

    LBINFO << "Async fetcher initialization finished" << std::endl;
    return sharedWindow;
}

void AsyncFetcher::setup( Window* window )
{
    _sharedWindow = initSharedContextWindow( window );
    start();
}

void AsyncFetcher::stop()
{
    if( !_sharedWindow )
        return;

    deleteTexture( 0 ); // exit async thread
    join();

    _sharedWindow->configExit();
    delete _sharedWindow;
    _sharedWindow = 0;
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
    eq::util::ObjectManager objects( glewGetContext( ));
    lunchbox::Bufferb textureData( 64*64*4 );
    LBINFO << "async fetcher initialized" << std::endl;

    bool running = true;
    lunchbox::sleep( 1000 ); // imitate loading of the first texture
    for( uint8_t* i = 0; running; ++i )
    {
        // generate new texture
        eq::util::Texture* tx = objects.newEqTexture( i, GL_TEXTURE_2D );
        tx->init( GL_RGBA8, 64, 64 );

        int j = 0;
        lunchbox::RNG rng;
        for( int y = 0; y < 64; ++y )
        {
            for( int x = 0; x < 64; ++x )
            {
                const GLbyte rnd = rng.get< uint8_t >() % 127;
                const GLbyte val = (x / 8) % 2 == (y / 8) % 2 ? rnd : 0;
                textureData[ j++ ] = val;
                textureData[ j++ ] = val;
                textureData[ j++ ] = val;
                textureData[ j++ ] = val;
            }
        }
        tx->upload( 64, 64, textureData.getData( ));
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
                objects.deleteEqTexture( keyToDelete );
            }
            else
                running = false;
        }
    }
    objects.deleteAll();
}

} //namespace eqAsync
