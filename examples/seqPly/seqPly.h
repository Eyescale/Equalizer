
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef SEQ_PLY_H
#define SEQ_PLY_H

#include <eq/sequel/sequel.h>

#include <vertexBufferDist.h>
#include <vertexBufferRoot.h>

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

/** The Sequel polygonal rendering example. */
namespace seqPly
{
    typedef mesh::VertexBufferRoot  Model;
    typedef eqPly::VertexBufferDist ModelDist;

    typedef std::vector< Model* >     Models;
    typedef std::vector< ModelDist* > ModelDists;

    typedef Models::const_iterator ModelsCIter;
    typedef ModelDists::const_iterator ModelDistsCIter;

    class Application : public seq::Application
    {
    public:
        Application() {}
        virtual ~Application() {}

        virtual bool init( const int argc, char** argv );

        virtual co::Object* createObject( const uint32_t type );
        virtual seq::Renderer* createRenderer();
    };

    typedef co::base::RefPtr< Application > ApplicationPtr;
}

#endif // SEQ_PLY_H

