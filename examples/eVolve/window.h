
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
            { return getObjectManager()->getList( key ); }
        GLuint newDisplayList( const void* key )
            { return getObjectManager()->newList( key ); }

        void getLogoTexture( GLuint& id, vmml::Vector2i& size ) const
            { id = _logoTexture; size = _logoSize; }

        GLEWContext* glewGetContext() { return getObjectManager()->glewGetContext(); }

    protected:
        virtual ~Window() {}
        virtual bool configInit( const uint32_t initID );
        virtual bool configInitGL( const uint32_t initID );
        virtual void swapBuffers();

    private:
        GLuint         _logoTexture;
        vmml::Vector2i _logoSize;

        void _loadLogo();
    };
}

#endif // EVOLVE_WINDOW_H
