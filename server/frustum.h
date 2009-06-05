
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_FRUSTUM_H
#define EQSERVER_FRUSTUM_H

#include <eq/client/frustum.h>     // base class

namespace eq
{
namespace server
{
    struct FrustumData;

    /** 
     * Extends the eq::Frustum to update server-side generic frustum data.
     */
    class Frustum : public eq::Frustum
    {
    public:
        Frustum( FrustumData& data );
        Frustum( const Frustum& from, FrustumData& data );
        virtual ~Frustum(){}
        
        /** Set the frustum using a wall description. */
        void setWall( const eq::Wall& wall );
        
        /** Set the frustum using a projection description. */
        void setProjection( const eq::Projection& projection );

        /** Set the eye separation. */
        void setEyeBase( const float eyeBase );

    protected:

    private:
        FrustumData& _data;

        /** Update the frustum (wall/projection). */
        void _updateFrustum();
    };

    std::ostream& operator << ( std::ostream& os, const Frustum* frustum );
}
}
#endif // EQ_FRUSTUM_H
