
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

#include <eq/gl.h>
#include <eq/windowSystem.h>

namespace eq
{
namespace util
{
namespace detail
{
class BitmapFont
{
public:
    BitmapFont( util::ObjectManager& om_, const void* key_ )
        // We create a new shared object manager. Typically we are exited by
        // the last user, at which point the given OM may have been deleted
        : om( om_ )
        , key( key_ )
    {}

    util::ObjectManager om;
    const void* key;
};
}

BitmapFont::BitmapFont( ObjectManager& om, const void* key )
    : _impl( new detail::BitmapFont( om, key ))
{
}

BitmapFont::~BitmapFont()
{
    const GLuint lists = _impl->om.getList( _impl->key );
    if( lists != ObjectManager::INVALID )
        LBWARN << "OpenGL BitmapFont was not freed" << std::endl;
    delete _impl;
}

bool BitmapFont::init( const WindowSystem& ws, const std::string& name,
                       const uint32_t size )
{
    return ws.setupFont( _impl->om, _impl->key, name, size );
}

void BitmapFont::exit()
{
    GLuint lists = _impl->om.getList( _impl->key );
    if( lists != ObjectManager::INVALID )
        _impl->om.deleteList( _impl->key );
}

void BitmapFont::draw( const std::string& text ) const
{
    const GLuint lists = _impl->om.getList( _impl->key );
    if( lists != ObjectManager::INVALID )
    {
        glListBase( lists );
        glCallLists( GLsizei( text.size( )), GL_UNSIGNED_BYTE, text.c_str( ));
        glListBase( 0 );
    }
}

}
}
