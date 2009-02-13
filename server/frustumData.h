
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_FRUSTUMDATA_H
#define EQSERVER_FRUSTUMDATA_H

#include <eq/base/base.h>
#include <eq/client/eye.h>   // EYE enum
#include <vmmlib/matrix4.h>  // member
#include <vmmlib/vector3.h>  // member

namespace eq
{
    class Projection;
    class Wall;

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
        //*{
        /** Update the frustum data using the given projection. */
        void applyProjection( const eq::Projection& projection );

        /** Update the frustum data using the given wall. */
        void applyWall( const eq::Wall& wall );

        /** Update the frustum data using the given head parameters. */
        void applyHead( const vmml::Matrix4f& headMatrix, const float eyeBase );
        //*}

        /** @name Data Access. */
        //*{
        /** @return the frustum plane transformation */
        const vmml::Matrix4f& getTransform() const { return _xfm; }

        /** @return the frustum plane width */
        float getWidth() const { return _width; }

        /** @return the frustum plane height */
        float getHeight() const { return _height; }

        /** @return the position of an eye in world-space coordinates. */
        const vmml::Vector3f& getEyePosition( const eq::Eye eye ) const
            { return _eyes[ eye ]; }
        //*}

    private:
        float _width;
        float _height;
        vmml::Matrix4f _xfm;
        vmml::Vector3f _eyes[eq::EYE_ALL];
    };

    std::ostream& operator << ( std::ostream& os, const FrustumData& ); 
}
}
#endif // EQ_FRUSTUMDATA_H
