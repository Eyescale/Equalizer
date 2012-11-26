
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

#ifndef EQASYNC_ASYNC_FETCHER_H
#define EQASYNC_ASYNC_FETCHER_H

#include <eq/eq.h>

namespace eqAsync
{

/**
 *  Structure to associate OpenGL texture ids with an external key.
 */
struct TextureId
{
    TextureId( const GLuint id_ = 0, const void* key_ = 0 )
            : id( id_ ), key( key_ ){};

    GLuint id;       // OpenGL texture id
    const void* key; // Object manager key; used to delete textures
};

class Window;

/**
 *  Asynchronous fetching thread. Creates and supplies new textures to the main rendering pipe.
 */
class AsyncFetcher : public lunchbox::Thread
{
public:
    typedef eq::util::ObjectManager< int > ObjectManager;

    AsyncFetcher();
    ~AsyncFetcher();

    virtual void run();
    void setup( Window* window );

    TextureId getTextureId()               { return _outQueue.pop().id;      }
    bool tryGetTextureId( TextureId& val ) { return _outQueue.tryPop( val ); }
    void deleteTexture( const void* key )  { _inQueue.push( key );           }

    const GLEWContext* glewGetContext() const;

private:
    Window*                        _window;
    lunchbox::MTQueue<const void*> _inQueue;       // textures to delete
    lunchbox::MTQueue<TextureId>   _outQueue;      // generated textures
    eq::ObjectManager*             _objectManager;
    eq::SystemWindow*              _sharedWindow;
    GLbyte*                        _tmpTexture;    // temporal texture storage
};

} 

#endif //EQASYNC_ASYNC_FETCHER_H
