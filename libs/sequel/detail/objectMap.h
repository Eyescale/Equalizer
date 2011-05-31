
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include <eq/sequel/objectType.h> // used inline
#include <eq/sequel/types.h>
#include <co/serializable.h>      // base class

namespace seq
{
namespace detail
{
    /** Central distributed object registry. */
    class ObjectMap : public co::Serializable
    {
    public:
        ObjectMap( ObjectFactory& factory );
        ~ObjectMap();

        bool register_( co::Object* object, const uint32_t type );
        co::Object* get( const uint128_t& identifier, co::Object* instance=0 );

        void setInitData( co::Object* object );
        void setFrameData( co::Object* object );

        co::Object* getInitData() { return get( _initData ); }
        co::Object* getFrameData() { return get( _frameData ); }

    protected:
        virtual bool isDirty() const;
        virtual uint32_t commitNB( const uint32_t incarnation );

        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        ObjectFactory& _factory; //!< The 'parent' user
        uint128_t _initData;
        uint128_t _frameData;

        /** The changed parts of the object since the last serialize(). */
        enum DirtyBits
        {
            DIRTY_ADDED       = co::Serializable::DIRTY_CUSTOM << 0, // 1
            DIRTY_CHANGED     = co::Serializable::DIRTY_CUSTOM << 1, // 2
            DIRTY_INITDATA    = co::Serializable::DIRTY_CUSTOM << 2, // 4
            DIRTY_FRAMEDATA   = co::Serializable::DIRTY_CUSTOM << 3  // 8
        };

        struct Entry //!< One object map item
        {
            Entry() : instance( 0 ), type( OBJECTTYPE_NONE ) {}
            Entry( const uint128_t& v, co::Object* i, const uint32_t t );

            uint128_t version;    //!< The current version of the object
            co::Object* instance; //!< The object instance, if attached
            uint32_t type;        //!< The object class id
        };

        typedef stde::hash_map< uint128_t, Entry > Map;
        typedef Map::iterator MapIter;
        typedef Map::const_iterator MapCIter;
        //typedef std::vector< Entry > Entries;

        co::base::Lockable< Map, co::base::SpinLock > _map; //!< the actual map

        /** Master objects registered with this instance. */
        co::base::Lockable< co::Objects, co::base::SpinLock > _masters;

        /** Added master objects since the last commit. */
        co::base::Lockable< co::ObjectVersions, co::base::SpinLock > _added;

        /** Changed master objects since the last commit. */
        co::base::Lockable< co::ObjectVersions, co::base::SpinLock > _changed;

        /** Commit and note new master versions. */
        void _commitMasters( const uint32_t incarnation );
    };
}
}
#endif // EQSEQUEL_DETAIL_OBJECTMAP_H
