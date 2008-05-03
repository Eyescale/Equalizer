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
    /** eqNet::Object to distribute a model, holds a VertexBufferBase node. */
    class VertexBufferDist : public eqNet::Object
    {
    public:
        VertexBufferDist( const mesh::VertexBufferRoot* root );
        virtual ~VertexBufferDist();

        void registerTree( eqNet::Session* session );
        void deregisterTree();

        static mesh::VertexBufferRoot* mapModel( eqNet::Session* session,
                                                 const uint32_t modelID );

    protected:
        VertexBufferDist( const mesh::VertexBufferRoot* root,
                          const mesh::VertexBufferBase* node );

        virtual void getInstanceData( eqNet::DataOStream& os );
        virtual void applyInstanceData( eqNet::DataIStream& is );

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
