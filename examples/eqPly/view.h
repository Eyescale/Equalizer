
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

        void setIdleSteps( const uint32_t steps );
        uint32_t getIdleSteps() const { return _idleSteps; }

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
            DIRTY_IDLE        = eq::View::DIRTY_CUSTOM << 1
        };

    private:
        uint32_t _modelID;
        uint32_t _idleSteps;
    };
}

#endif // EQ_PLY_VIEW_H
