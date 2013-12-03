
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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


#ifndef EQ_PLY_VERTEXBUFFERDIST_H
#define EQ_PLY_VERTEXBUFFERDIST_H

#include "api.h"
#include "vertexBufferRoot.h"

#include <eq/eq.h>

namespace eqPly
{
    /** co::Object to distribute a model, holds a VertexBufferBase node. */
    class VertexBufferDist : public co::Object
    {
    public:
        PLYLIB_API VertexBufferDist();
        PLYLIB_API VertexBufferDist( mesh::VertexBufferRoot* root );
        PLYLIB_API virtual ~VertexBufferDist();

        PLYLIB_API void registerTree( co::LocalNodePtr node );
        PLYLIB_API void deregisterTree();

        PLYLIB_API mesh::VertexBufferRoot* loadModel( co::NodePtr master,
                                           co::LocalNodePtr localNode,
                                           const eq::UUID& modelID );

    protected:
        PLYLIB_API VertexBufferDist( mesh::VertexBufferRoot* root,
                          mesh::VertexBufferBase* node );

        PLYLIB_API virtual void getInstanceData( co::DataOStream& os );
        PLYLIB_API virtual void applyInstanceData( co::DataIStream& is );

    private:
        mesh::VertexBufferRoot* _root;
        mesh::VertexBufferBase* _node;
        VertexBufferDist* _left;
        VertexBufferDist* _right;
        bool _isRoot;
    };
}


#endif // EQ_PLY_VERTEXBUFFERDIST_H
