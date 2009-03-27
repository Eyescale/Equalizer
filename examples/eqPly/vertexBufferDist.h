
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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


#ifndef EQPLY_VERTEXBUFFERDIST_H
#define EQPLY_VERTEXBUFFERDIST_H

#include "vertexBufferRoot.h"

#include <eq/eq.h>

namespace eqPly 
{
    /** eq::net::Object to distribute a model, holds a VertexBufferBase node. */
    class VertexBufferDist : public eq::net::Object
    {
    public:
        VertexBufferDist();
        VertexBufferDist( const mesh::VertexBufferRoot* root );
        virtual ~VertexBufferDist();

        void registerTree( eq::net::Session* session );
        void deregisterTree();

        mesh::VertexBufferRoot* mapModel( eq::net::Session* session,
                                          const uint32_t modelID );
        void unmapTree();

    protected:
        VertexBufferDist( const mesh::VertexBufferRoot* root,
                          const mesh::VertexBufferBase* node );

        virtual void getInstanceData( eq::net::DataOStream& os );
        virtual void applyInstanceData( eq::net::DataIStream& is );

    private:
        const mesh::VertexBufferRoot* _root;
        const mesh::VertexBufferBase* _node;
        bool                          _isRoot;

        VertexBufferDist* _left;
        VertexBufferDist* _right;
    };
}


#endif // EQPLY_VERTEXBUFFERDIST_H
