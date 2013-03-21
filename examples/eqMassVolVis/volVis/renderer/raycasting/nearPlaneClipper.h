
#ifndef NEAR_PLANE_CLIPPER_H
#define NEAR_PLANE_CLIPPER_H

#include <msv/types/plane.h>

#include <boost/array.hpp>

namespace vmml {
    template< size_t M, size_t N, typename T > class matrix;
}

namespace eq {
namespace fabric {
    typedef vmml::matrix< 4, 4, float >  Matrix4f;
}}


namespace massVolVis
{

class RenderNode;

class NearPlaneClipper
{
public:
    NearPlaneClipper(){}

    void setup( const eq::fabric::Matrix4f& projectionMV );

    size_t intersectBrick( const RenderNode& renderNode );

    const Vec3_f& getCoordinateForPoint( size_t index );
    const Vec3_f& getColorForPoint(      size_t index );

private:
    class VectorsCmp;
    struct CoordinatePlusColor
    {
        CoordinatePlusColor(){}
        CoordinatePlusColor( const Vec3_f& coord_, const Vec3_f& color_ )
            : coord( coord_ ), color( color_ ){}

        Vec3_f coord;
        Vec3_f color;
    };

    Plane_d _nearPlane;

    boost::array<Vec3_d, 8> _vertices;
    boost::array<double, 8> _distancesToNearPlane;

    boost::array<CoordinatePlusColor,6> _result;
};

} // namespace massVolVis

#endif //NEAR_PLANE_CLIPPER_H
