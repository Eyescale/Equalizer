
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

#include <eq/fabric/projection.h> // member
#include <eq/fabric/wall.h>       // member
#include <eq/fabric/base.h>       // decl

namespace eq
{
namespace net
{
    class DataOStream;
    class DataIStream;
}

namespace fabric
{
    /** A distributed object for frustum data. */
    class Frustum
    {
    public:
        /** Construct a new frustum. @version 1.0 */
        EQ_FABRIC_DECL Frustum();
        
        /** Destruct the frustum. @version 1.0 */
        EQ_FABRIC_DECL virtual ~Frustum();

        /** The type of the last specified frustum. @version 1.0 */
        enum Type
        {
            TYPE_NONE,        //!< No frustum has been specified
            TYPE_WALL,        //!< A wall description has been set last
            TYPE_PROJECTION   //!< A projection description has been set last
        };

        /** Set the frustum using a wall description. @version 1.0 */
        EQ_FABRIC_DECL virtual void setWall( const Wall& wall );
        
        /** Set the frustum using a projection description. @version 1.0 */
        EQ_FABRIC_DECL virtual void setProjection( const Projection& projection );

        /** @return the last specified frustum as a wall. @version 1.0 */
        const Wall& getWall() const { return _data.wall; }

        /** @return the last specified frustum as a projection. @version 1.0 */
        const Projection& getProjection() const { return _data.projection; }

        /** @return the type of the latest specified frustum. @version 1.0 */
        Type getCurrentType() const { return _data.current; }

        /** Set the last specified frustum to TYPE_NONE. @version 1.0 */
        EQ_FABRIC_DECL virtual void unsetFrustum();

        EQFABRIC_API virtual void backup(); //!< @internal
        EQFABRIC_API virtual void restore(); //!< @internal

    protected:

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

    EQ_FABRIC_DECL std::ostream& operator << ( std::ostream& os, const Frustum& );
    EQ_FABRIC_DECL net::DataOStream& operator << (net::DataOStream&, const Frustum&);
    EQ_FABRIC_DECL net::DataIStream& operator >> (net::DataIStream&, Frustum& );
}
}
#endif // EQFABRIC_FRUSTUM_H
