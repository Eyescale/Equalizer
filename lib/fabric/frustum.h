
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

#ifndef EQFABRIC_FRUSTUM_H
#define EQFABRIC_FRUSTUM_H

#include <eq/fabric/object.h>     // base class
#include <eq/fabric/projection.h> // member
#include <eq/fabric/wall.h>       // member

namespace eq
{
namespace fabric
{
    /** A distributed object for frustum data. */
    class Frustum : public Object
    {
    public:
        /** Construct a new frustum. @version 1.0 */
        EQ_EXPORT Frustum();
        
        /** Destruct the frustum. @version 1.0 */
        EQ_EXPORT virtual ~Frustum();

        /** The type of the last specified frustum. @version 1.0 */
        enum Type
        {
            TYPE_NONE,        //!< No frustum has been specified
            TYPE_WALL,        //!< A wall description has been set last
            TYPE_PROJECTION   //!< A projection description has been set last
        };

        /** Set the frustum using a wall description. @version 1.0 */
        EQ_EXPORT void setWall( const Wall& wall );
        
        /** Set the frustum using a projection description. @version 1.0 */
        EQ_EXPORT void setProjection( const Projection& projection );

        /** @return the last specified frustum as a wall. @version 1.0 */
        EQ_EXPORT const Wall& getWall() const { return _data.wall; }

        /** @return the last specified frustum as a projection. @version 1.0 */
        EQ_EXPORT const Projection& getProjection() const
            { return _data.projection; }

        /** @return the type of the latest specified frustum. @version 1.0 */
        EQ_EXPORT Type getCurrentType() const { return _data.current; }

        /** Set the last specified frustum to TYPE_NONE. @version 1.0 */
        EQ_EXPORT void unsetFrustum();

        virtual void backup(); //!< @internal
        virtual void restore(); //!< @internal

    protected:
        /** @sa Object::serialize() */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint64_t dirtyBits );
        /** @sa Object::deserialize() */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        /** The changed parts of the frustum since the last pack(). */
        enum DirtyBits
        {
            DIRTY_TYPE       = Object::DIRTY_CUSTOM << 0,
            DIRTY_WALL       = Object::DIRTY_CUSTOM << 1,
            DIRTY_PROJECTION = Object::DIRTY_CUSTOM << 2,
            DIRTY_CUSTOM     = Object::DIRTY_CUSTOM << 5,
        };

    private:
        struct BackupData
        {
            BackupData() : current( TYPE_NONE ) {}

            /** The frustum description as a wall. */
            Wall wall;

            /** The frustum description as a projection. */
            Projection projection;

            /** The type of the last specified frustum description. */
            Type current;
        }
            _data, _backup;

        union // placeholder for binary-compatible changes
        {
            char dummy[16];
        };
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Frustum& );
}
}
#endif // EQFABRIC_FRUSTUM_H
