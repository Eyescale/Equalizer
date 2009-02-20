
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_VIEW_H
#define EQ_PLY_VIEW_H

#include <eq/eq.h>

#include "vertexBufferState.h"
#include <string>

namespace eqPly
{
    class View : public eq::View
    {
    public:
        View();
        virtual ~View();

        void setModelID( const uint32_t id );
        uint32_t getModelID() const { return _modelID; }

    protected:
        /** @sa eq::View::serialize() */
        virtual void serialize( eq::net::DataOStream& os,
                                const uint64_t dirtyBits );
        /** @sa eq::View::deserialize() */
        virtual void deserialize( eq::net::DataIStream& is, 
                                  const uint64_t dirtyBits );

        /** The changed parts of the view. */
        enum DirtyBits
        {
            DIRTY_MODEL       = eq::View::DIRTY_CUSTOM << 0,
        };

    private:
        uint32_t _modelID;
    };
}

#endif // EQ_PLY_VIEW_H
