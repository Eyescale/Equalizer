
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
