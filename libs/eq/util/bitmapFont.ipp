
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace eq
{
namespace util
{

template< class OMT >
BitmapFont< OMT >::BitmapFont( ObjectManager< OMT >& gl, const OMT& key )
        // We create a new shared object manager. Typically we are exited by
        // the last user, at which point the given OM may have been deleted
        : _gl( &gl )
        , _key( key )
{
}

template< class OMT >
BitmapFont< OMT >::~BitmapFont()
{
    const GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager< OMT >::INVALID )
        LBWARN << "OpenGL BitmapFont was not freed" << std::endl;
}

template< class OMT >
bool BitmapFont< OMT >::init( const WindowSystem ws, const std::string& name,
                              const uint32_t size )
{
    return ws.setupFont( _gl, _key, name, size );
}

template< class OMT >
void BitmapFont< OMT >::exit()
{
    GLuint lists = _gl.getList( _key );
    if( lists != ObjectManager< OMT >::INVALID )
        _gl.deleteList( _key );
}

template< class OMT >
void BitmapFont< OMT >::draw( const std::string& text ) const
{
    const GLuint lists = _gl.getList( _key );
    LBASSERTINFO( lists != ObjectManager< OMT >::INVALID, 
                  "Font not initialized" );

    if( lists != ObjectManager< OMT >::INVALID )
    {
        glListBase( lists );
        glCallLists( GLsizei( text.size( )), GL_UNSIGNED_BYTE, text.c_str( ));
        glListBase( 0 );
    }
}
}
}
