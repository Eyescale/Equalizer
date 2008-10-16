
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEWDATA_H
#define EQSERVER_VIEWDATA_H

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
     * Data derived from eq::View, in a general, optimized format used for
     * frustum calculations during rendering.
     */
    class ViewData
    {
    public:
        ViewData() : _width(0.f), _height(0.f) {}

        bool isValid() const { return (_width!=0.f && _height!=0.f); }
        void invalidate()    { _width = 0.f; _height = 0.f; }

        /** @name Data Update. */
        //*{
        /** Update the view data using the given projection. */
        void applyProjection( const eq::Projection& projection );

        /** Update the view data using the given wall. */
        void applyWall( const eq::Wall& wall );

        /** Update the view data using the given head parameters. */
        void applyHead( const vmml::Matrix4f& headMatrix, const float eyeBase );
        //*}

        /** @name Data Access. */
        //*{
        /** @return the view plane transformation */
        const vmml::Matrix4f& getViewTransform() const { return _xfm; }

        /** @return the view plane width */
        float getViewWidth() const { return _width; }

        /** @return the view plane height */
        float getViewHeight() const { return _height; }

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

    std::ostream& operator << ( std::ostream& os, const ViewData& viewData ); 
}
}
#endif // EQ_VIEWDATA_H
