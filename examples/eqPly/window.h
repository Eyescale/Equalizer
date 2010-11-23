
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
 */

#ifndef EQ_PLY_WINDOW_H
#define EQ_PLY_WINDOW_H

#include <eq/eq.h>

#include "vertexBufferState.h"
#include <string>

namespace eqPly
{
    /**
     * A window represent an OpenGL drawable and context
     *
     * Manages OpenGL-specific data, i.e., it creates the logo texture during
     * initialization and holds a state object for GL object creation. It
     * initializes the OpenGL state and draws the statistics overlay.
     */
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) 
                : eq::Window( parent ), _state( 0 ), _logoTexture( 0 ) {}

        const eq::util::Texture* getLogoTexture() const { return _logoTexture; }
        VertexBufferState& getState() { return *_state; }
        
    protected:
        virtual ~Window() {}
        virtual bool configInitSystemWindow( const eq::uint128_t& initID );
        virtual bool configInitGL( const eq::uint128_t& initID );
        virtual bool configExitGL();
        virtual void frameStart( const eq::uint128_t& frameID,
                                 const uint32_t frameNumber );

    private:
        VertexBufferState* _state;
        eq::util::Texture* _logoTexture;

        void _loadLogo();
        void _loadShaders();
    };
}

#endif // EQ_PLY_WINDOW_H
