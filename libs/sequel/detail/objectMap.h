
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

#include <eq/sequel/types.h>
#include <eq/fabric/serializable.h> // base class

namespace seq
{
namespace detail
{
    /** Central distributed object registry. */
    class ObjectMap : public eq::fabric::Serializable
    {
    public:
        ObjectMap( Config* config );
        ~ObjectMap();

        bool register_( co::Object* object );
        bool map( co::Object* object, const uint128_t& identifier );

    protected:
        virtual bool isDirty() const;
        virtual uint32_t commitNB( const uint32_t incarnation );

        virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is,
                                  const uint64_t dirtyBits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        Config* const _config; //!< The parent config
        
        /** The changed parts of the object since the last serialize(). */
        enum DirtyBits
        {
            DIRTY_ADDED       = Serializable::DIRTY_CUSTOM << 0, // 1
            DIRTY_CHANGED     = Serializable::DIRTY_CUSTOM << 1, // 2
            DIRTY_OBJECT_BITS = DIRTY_ADDED
        };

        struct Entry //!< One object map item
        {
            Entry() : instance( 0 ) {}

            uint128_t version;    //!< The current version of the object
            co::Object* instance; //!< The object instance, if attached
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
