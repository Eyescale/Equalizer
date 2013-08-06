
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EVOLVE_WINDOW_H
#define EVOLVE_WINDOW_H

#include <eq/eq.h>

namespace eVolve
{
class Window : public eq::Window
{
public:
    Window( eq::Pipe* parent ) : eq::Window( parent ), _logoTexture( 0 ) {}

    // display list cache (windows share the context and object manager)
    GLuint getDisplayList( const void* key )
        { return getObjectManager().getList( key ); }
    GLuint newDisplayList( const void* key )
        { return getObjectManager().newList( key ); }

    const eq::util::Texture* getLogoTexture() const { return _logoTexture; }

protected:
    virtual ~Window() {}
    virtual bool configInit( const eq::uint128_t& initID );
    virtual bool configInitGL( const eq::uint128_t& initID );
    virtual void swapBuffers();

private:
    eq::util::Texture* _logoTexture;

    void _loadLogo();
};
}

#endif // EVOLVE_WINDOW_H
