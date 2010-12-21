
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_FRUSTUMDATA_H
#define EQSERVER_FRUSTUMDATA_H

#include <co/base/os.h>
#include <eq/fabric/eye.h>   // EYE enum
#include <eq/fabric/wall.h>  // Wall::Type enum
#include <vmmlib/matrix.hpp>  // member
#include <vmmlib/vector.hpp>  // member

namespace eq
{

namespace server
{
    /** 
     * Data derived from eq::Frustum, in a general, optimized format used for
     * frustum calculations during rendering.
     */
    class FrustumData
    {
    public:
        FrustumData();

        bool isValid() const { return (_width!=0.f && _height!=0.f); }
        void invalidate()    { _width = 0.f; _height = 0.f; }

        /** @name Data Update. */
        //@{
        /** Update the frustum data using the given projection. */
        void applyProjection( const eq::Projection& projection );

        /** Update the frustum data using the given wall. */
        void applyWall( const eq::Wall& wall );
        //@}

        /** @name Data Access. */
        //@{
        /** @return the frustum plane transformation */
        const Matrix4f& getTransform() const { return _xfm; }

        /** @return the frustum plane width */
        float getWidth() const { return _width; }

        /** @return the frustum plane height */
        float getHeight() const { return _height; }

        /** @return the projection type. */
        eq::Wall::Type getType() const { return _type; }
        //@}

    private:
        float _width;
        float _height;
        Matrix4f _xfm;
        eq::Wall::Type _type;
    };

    std::ostream& operator << ( std::ostream& os, const FrustumData& ); 
}
}
#endif // EQ_FRUSTUMDATA_H
