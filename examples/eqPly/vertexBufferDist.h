/*  
 *   Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com>
 *   All rights reserved.  
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
        VertexBufferDist( const mesh::VertexBufferRoot* root );
        virtual ~VertexBufferDist();

        void registerTree( eq::net::Session* session );
        void deregisterTree();

        static mesh::VertexBufferRoot* mapModel( eq::net::Session* session,
                                                 const uint32_t modelID );

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

        void _unmapTree();
    };
}


#endif // EQPLY_VERTEXBUFFERDIST_H
