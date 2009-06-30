
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "vertexBufferBase.h"
#include "vertexBufferState.h"

namespace mesh
{
void VertexBufferBase::renderBoundingSphere(VertexBufferState& state ) const
{
    GLuint displayList = state.getDisplayList( &_boundingSphere );

    if( displayList == state.INVALID )
    {
        displayList = state.newDisplayList( &_boundingSphere );
        glNewList( displayList, GL_COMPILE );

        const float x  = _boundingSphere.x();
        const float y  = _boundingSphere.y();
        const float z  = _boundingSphere.z();
        const float x1 = x - _boundingSphere.w();
        const float x2 = x + _boundingSphere.w();
        const float y1 = y - _boundingSphere.w();
        const float y2 = y + _boundingSphere.w();
        const float z1 = z - _boundingSphere.w();
        const float z2 = z + _boundingSphere.w();
        const float size = _boundingSphere.w();

//        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        glNormal3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( x1, y - size, z - size );
        glVertex3f( x1, y + size, z - size );
        glVertex3f( x1, y + size, z + size );
        glVertex3f( x1, y - size, z + size );
        glNormal3f( -1.0f, 0.0f, 0.0f );
        glVertex3f( x2, y - size, z - size );
        glVertex3f( x2, y - size, z + size );
        glVertex3f( x2, y + size, z + size );
        glVertex3f( x2, y + size, z - size );

        glNormal3f( 0.0f, -1.0f, 0.0f );
        glVertex3f( x - size, y2, z - size );
        glVertex3f( x + size, y2, z - size );
        glVertex3f( x + size, y2, z + size );
        glVertex3f( x - size, y2, z + size );
        glNormal3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( x - size, y1, z - size );
        glVertex3f( x - size, y1, z + size );
        glVertex3f( x + size, y1, z + size );
        glVertex3f( x + size, y1, z - size );

        glNormal3f( 0.0f, 0.0f, -1.0f );
        glVertex3f( x - size, y - size, z2 );
        glVertex3f( x - size, y + size, z2 );
        glVertex3f( x + size, y + size, z2 );
        glVertex3f( x + size, y - size, z2 );
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( x - size, y - size, z1 );
        glVertex3f( x + size, y - size, z1 );
        glVertex3f( x + size, y + size, z1 );
        glVertex3f( x - size, y + size, z1 );
        glEnd();
//        glEnable( GL_LIGHTING );

        glEndList();
    }

    glCallList( displayList );
}
}
