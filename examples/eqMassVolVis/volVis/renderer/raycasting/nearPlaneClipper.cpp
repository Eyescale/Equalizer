
#include "nearPlaneClipper.h"

#include "../../LB/renderNode.h"

#include <msv/types/vmmlTypes.h>

#include <algorithm>

namespace massVolVis
{


void NearPlaneClipper::setup( const Matrix4f& projectionMV )
{
    // get normalized near plane
    Vector4d plane = projectionMV.get_row( 2 ) +
                     projectionMV.get_row( 3 );
    plane *= 1.0 / sqrt( plane.x()*plane.x() +
                         plane.y()*plane.y() +
                         plane.z()*plane.z() );

    _nearPlane = Plane_d( plane.x(), plane.y(), plane.z(), plane.w() );

    // move near plane slightly ahead, so that when we will draw it, it will not be clipped
    _nearPlane.w -= 0.001;
//    _nearPlane.w -= 2.0;
}


const Vec3_f& NearPlaneClipper::getCoordinateForPoint( size_t index )
{
    assert( index < _result.size() );
    return _result[ index ].coord;
}


const Vec3_f& NearPlaneClipper::getColorForPoint( size_t index )
{
    assert( index < _result.size() );
    return _result[ index ].color;
}


// Arranging of points into a convex hull is taken from here:
//   http://www.asawicki.info/news_1428_finding_polygon_of_plane-aabb_intersection.html
class NearPlaneClipper::VectorsCmp
{
public:
    VectorsCmp( Vec3_f origin, Vec3_f plainN ) : _origin(origin), _plainN(plainN) {}

    bool operator()( const NearPlaneClipper::CoordinatePlusColor& l,
                     const NearPlaneClipper::CoordinatePlusColor& r )
    {
        return _plainN.dot( (l.coord-_origin).cross(r.coord-_origin) ) < 0;
    }

private:
    Vec3_f _origin;
    Vec3_f _plainN;
};


namespace
{
int _edges[12][2] = {{0,1}, {0,2}, {1,3}, {2,3},
                     {0,4}, {1,5}, {2,6}, {3,7},
                     {4,5}, {4,6}, {5,7}, {6,7} };

Vec3_f _colors[8] = { Vec3_f(0,0,0),
                      Vec3_f(1,0,0),
                      Vec3_f(0,1,0),
                      Vec3_f(1,1,0),
                      Vec3_f(0,0,1),
                      Vec3_f(1,0,1),
                      Vec3_f(0,1,1),
                      Vec3_f(1,1,1)};
} // namespace


size_t NearPlaneClipper::intersectBrick( const RenderNode& renderNode )
{
    if( renderNode.fullyVsible )
        return 0;

    const Vec3_f& s = renderNode.coords.s;
    const Vec3_f& e = renderNode.coords.e;

    _vertices[0].set( s.x, s.y, s.z );
    _vertices[1].set( e.x, s.y, s.z );
    _vertices[2].set( s.x, e.y, s.z );
    _vertices[3].set( e.x, e.y, s.z );
    _vertices[4].set( s.x, s.y, e.z );
    _vertices[5].set( e.x, s.y, e.z );
    _vertices[6].set( s.x, e.y, e.z );
    _vertices[7].set( e.x, e.y, e.z );

    boost::array<double, 8>& _dist = _distancesToNearPlane;

    double minDist = _nearPlane.distanceToPoint( _vertices[0] );
    double maxDist = minDist;
    _dist[0] = minDist;
    for( int i = 1; i < 8; ++i )
    {
        double dist = _nearPlane.distanceToPoint( _vertices[i] );
        _dist[i] = dist;
        if( dist < minDist )
            minDist = dist;
        else
            if( dist > maxDist )
                maxDist = dist;
    }
    if( minDist*maxDist >= 0.0 )
        return 0;

    // find edges that are crossing the plane
    size_t validVertices = 0;
    for( int i = 0; i < 12; ++i )
    {
        int p1 = _edges[i][0];
        int p2 = _edges[i][1];
        if( _dist[p1]*_dist[p2] < 0 )
        {
            if( _dist[p1] > 0 )
                std::swap( p1, p2 );// now _d[p1] is negative and _d[p2] is positive

            double l = _dist[p1] / ( _dist[p1] - _dist[p2] );

            Vec3_f pos   = _vertices[p1] + (_vertices[p2]-_vertices[p1])*l;
            Vec3_f color = _colors[  p1] + (_colors[  p2]-_colors[  p1])*l;

            _result[validVertices++] = CoordinatePlusColor( pos, color );
        }
    }

    if( validVertices < 3 )
        return 0;

    std::sort( _result.begin()+1, _result.begin()+validVertices, VectorsCmp( _result[0].coord, _nearPlane.normal ));

    return validVertices;
}


} // namespace massVolVis
