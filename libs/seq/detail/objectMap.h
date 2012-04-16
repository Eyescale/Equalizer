
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSEQUEL_DETAIL_OBJECTMAP_H
#define EQSEQUEL_DETAIL_OBJECTMAP_H

#include <seq/objectType.h> // used inline
#include <seq/types.h>
#include <co/objectMap.h>      // base class

namespace seq
{
namespace detail
{
    /** Central distributed object registry. */
    class ObjectMap : public co::ObjectMap
    {
    public:
        ObjectMap( eq::Config& config, co::ObjectFactory& factory );
        ~ObjectMap();

        void setInitData( co::Object* object );
        void setFrameData( co::Object* object );

        co::Object* getInitData( co::Object* object )
            { return get( _initData, object ); }
        co::Object* getFrameData() { return get( _frameData ); }

    protected:
        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

    private:
        uint128_t _initData;
        uint128_t _frameData;

        /** The changed parts of the object since the last serialize(). */
        enum DirtyBits
        {
            DIRTY_INITDATA    = co::ObjectMap::DIRTY_CUSTOM << 0, // 4
            DIRTY_FRAMEDATA   = co::ObjectMap::DIRTY_CUSTOM << 1  // 8
        };
    };
}
}
#endif // EQSEQUEL_DETAIL_OBJECTMAP_H
