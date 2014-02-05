
/* Copyright (c) 2008-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "bitmapFont.h"
#include "objectManager.h"

#include <eq/client/gl.h>
#include <eq/client/windowSystem.h>

namespace eq
{
namespace util
{

BitmapFont::BitmapFont( ObjectManager& gl, const void* key )
        // We create a new shared object manager. Typically we are exited by
        // the last user, at which point the given OM may have been deleted
        : _gl( gl )
        , _key( key )
{
}

BitmapFont::~BitmapFont()
{
    const GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager::INVALID )
        LBWARN << "OpenGL BitmapFont was not freed" << std::endl;
}

bool BitmapFont::init( const WindowSystem& ws, const std::string& name,
                       const uint32_t size )
{
    return ws.setupFont( _gl, _key, name, size );
}

void BitmapFont::exit()
{
    GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager::INVALID )
        _gl.deleteList( _key );
}

void BitmapFont::draw( const std::string& text ) const
{
    const GLuint lists = _gl.getList( _key );
    LBASSERTINFO( lists != ObjectManager::INVALID,
                  "Font not initialized" );

    if( lists != ObjectManager::INVALID )
    {
        glListBase( lists );
        glCallLists( GLsizei( text.size( )), GL_UNSIGNED_BYTE, text.c_str( ));
        glListBase( 0 );
    }
}

}
}
