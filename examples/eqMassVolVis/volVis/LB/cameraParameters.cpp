
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "cameraParameters.h"

#include <msv/types/box.h>
#include <msv/types/limits.h>

namespace massVolVis
{


CameraParameters::CameraParameters( const Matrix4f& MV, const Matrix4f& projectionMV, const Vec2_f& viewportCenter, const Vec2_f& screenSize )
    : _MV( MV )
    , _projMV( projectionMV )
    , _screenSize( screenSize )
    , _viewportCenter( viewportCenter )
{
    Matrix3f MVIT;  // Model-View Inversed Transposed

    Matrix4f MVI;   // Model-View Inversed
    MV.inverse( MVI );
    Matrix3f( MVI ).transpose_to( MVIT );

    _xNormal = MVIT * Vector3f( 1.0, 0.0, 0.0 );
    _xNormal.normalize();

    _yNormal = MVIT * Vector3f( 0.0, 1.0, 0.0 );
    _yNormal.normalize();

    _zNormal = MVIT * Vector3f( 0.0, 0.0, 1.0 );
    _zNormal.normalize();

    _culler.setup( projectionMV );
}


bool CameraParameters::operator==( const CameraParameters& cp ) const
{
    return _MV          == cp._MV      &&
           _projMV      == cp._projMV  &&
           _screenSize  == cp._screenSize;
}


bool CameraParameters::operator==( const CameraParameters* cp ) const
{
    return _MV          == cp->_MV      &&
           _projMV      == cp->_projMV  &&
           _screenSize  == cp->_screenSize;
}


vmml::Visibility CameraParameters::computeVisibility( const Box_f& coords ) const
{
    BoundingSphere bs = coords.getBoundingSphere( );
    return _culler.test_sphere( Vector4f( bs.x, bs.y, bs.z, bs.r ));
}


Rect_i32 CameraParameters::computeScreenProjectionRect( const Box_f& coords ) const
{
    float x[] = {coords.s.x, coords.e.x};
    float y[] = {coords.s.y, coords.e.y};
    float z[] = {coords.s.z, coords.e.z};

    Rect_f res( Vec2_f( Limits< float >::max() ),
                Vec2_f( Limits< float >::min() ));

    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
            for( int k = 0; k < 2; ++k )
            {
                const Vector4f c( x[i], y[j], z[k], 1.f );
                Vector4f r = _projMV * c;
                r /= r.w() > 0.1 ? r.w() * 2.f : 0.2;
                res.expand( Vec2_f(r.x(), r.y()) );
            }
    return Rect_i32( _viewportCenter.x + res.s.x*_screenSize.x, _viewportCenter.y + res.s.y*_screenSize.y,
                     _viewportCenter.x + res.e.x*_screenSize.x, _viewportCenter.y + res.e.y*_screenSize.y );
}


namespace
{
inline Vector3f _transformPoint( const Vector4f& point, const Matrix4f& MV )
{
    const Vector4f p = MV * point;
    Vector3f pSub( p[ 0 ], p[ 1 ], p[ 2 ] );
    pSub.normalize();

    return pSub;
}


inline bool _normalProductSign( const Vector3f& normal, const Vector4f& point, const Matrix4f& MV )
{
    const Vector3f p = _transformPoint( point, MV );

    return normal.dot( p ) >= 0.0;
}
}// namespace


void CameraParameters::normalsProductsSigns( const Vector4f& point, bool& x, bool& y, bool& z ) const
{
    const Vector3f p = _transformPoint( point, _MV );

    x = _xNormal.dot( p ) >= 0;
    y = _yNormal.dot( p ) >= 0;
    z = _zNormal.dot( p ) >= 0;
}


bool CameraParameters::xNormalProductSign( const Vector4f& point ) const
{
    return _normalProductSign( _xNormal, point, _MV );
}


bool CameraParameters::yNormalProductSign( const Vector4f& point ) const
{
    return _normalProductSign( _yNormal, point, _MV );
}


bool CameraParameters::zNormalProductSign( const Vector4f& point ) const
{
    return _normalProductSign( _zNormal, point, _MV );
}


}

