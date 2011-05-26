/* Copyright (c) 2009-2011, Maxim Makhinya  <maxmah@gmail.com>
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

#ifndef EQ_EXAMPLE_ASYNC_H
#define EQ_EXAMPLE_ASYNC_H

#include "asyncFetcher.h"

#include <eq/eq.h>

namespace eqAsync
{

/* Simple Channel class */
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent ) : eq::Channel( parent ) {}

protected:
    virtual void frameDraw( const eq::uint128_t& spin );
};


/* Simple Window class that when initialized will call init of the 
   pipe to create a shared context */
class Window : public eq::Window
{
public:
    Window( eq::Pipe* parent ) : eq::Window( parent ) {}
    bool configInitGL( const eq::uint128_t& initID );
};



/* Simple Pipe class that creates a shared 
   window for async fetching */
class Pipe : public eq::Pipe
{
public:
    Pipe( eq::Node* parent ) : eq::Pipe( parent ), _initialized( false ), _txId( 0 ) {}

    void startAsyncFetcher( Window* wnd );

    AsyncFetcher& getAsyncFetcher() { return _asyncFetcher; }

    GLuint getTextureId() const { return _txId.id; }

protected:
        /* checks if new textures are avaliable */
        virtual void frameStart( const eq::uint128_t& frameID, 
                                 const uint32_t frameNumber );

        virtual bool configExit();
private:
    bool         _initialized;
    AsyncFetcher _asyncFetcher;
    TextureId    _txId;
};

}

#endif // EQ_EXAMPLE_ASYNC_H
