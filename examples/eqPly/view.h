
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_PLY_VIEW_H
#define EQ_PLY_VIEW_H

#include <eq/eq.h>

#include "vertexBufferState.h"
#include <plylib/vertexBufferState.h>

#include <string>

namespace eqPly
{
    class View : public eq::View
    {
    public:
        View( eq::Layout* parent );
        virtual ~View();

        void setModelID( const eq::UUID& id );
        eq::UUID getModelID() const { return _modelID; }

        void setIdleSteps( const int32_t steps );
        int32_t getIdleSteps() const { return _idleSteps; }

        void toggleEqualizer();

    private:
        class Proxy : public co::Serializable
        {
        public:
            Proxy( View* view ) : _view( view ) {}

        protected:
            /** The changed parts of the view. */
            enum DirtyBits
            {
                DIRTY_MODEL = co::Serializable::DIRTY_CUSTOM << 0,
                DIRTY_IDLE  = co::Serializable::DIRTY_CUSTOM << 1
            };

            virtual void serialize( co::DataOStream&, const uint64_t );
            virtual void deserialize( co::DataIStream&, const uint64_t );
            virtual void notifyNewVersion() { sync(); }

        private:
            View* const _view;
            friend class eqPly::View;
        };

        Proxy _proxy;
        friend class Proxy;
        eq::UUID _modelID;
        int32_t _idleSteps;
    };
}

#endif // EQ_PLY_VIEW_H
