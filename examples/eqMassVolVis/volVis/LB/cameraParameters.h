
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__CAMERA_PARAMETERS_H
#define MASS_VOL__CAMERA_PARAMETERS_H

#include <msv/types/rectangle.h>

#include <msv/types/vmmlTypes.h>

namespace massVolVis
{

template< typename T > struct Box;
typedef Box< float> Box_f;


class CameraParameters
{
public:
    CameraParameters(){};
    CameraParameters( const Matrix4f& MV, const Matrix4f& projectionMV, const Vec2_f& viewportCenter, const Vec2_f& screenSize );

    bool xNormalProductSign( const Vector4f& point ) const;
    bool yNormalProductSign( const Vector4f& point ) const;
    bool zNormalProductSign( const Vector4f& point ) const;

    void normalsProductsSigns( const eq::fabric::Vector4f& point, bool& x, bool& y, bool& z ) const;

    Rect_i32 computeScreenProjectionRect( const Box_f& coords ) const;

    vmml::Visibility computeVisibility( const Box_f& coords ) const;

    bool operator==( const CameraParameters& cp ) const;
    bool operator==( const CameraParameters* cp ) const;
private:

    Matrix4f _MV;           // Model-View Matrix
    Matrix4f _projMV;       // Projection Model-View Matrix
    Vec2_f   _screenSize;   // screen size in pixels
    Vec2_f   _viewportCenter;

    Vector3f _xNormal;
    Vector3f _yNormal;
    Vector3f _zNormal;

// TODO: fix vmmlib to make requests const
    mutable vmml::frustum_culler<float> _culler;
};


} //namespace massVolVis

#endif //MASS_VOL__ORDER_ESTIMATOR_H

