
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "vertexData.h"

#include <algorithm>

using namespace mesh;
using namespace std;

// Vector sorting functors for sorting along the three major axis
struct XAxisSort
{
    XAxisSort( VertexData* data ) : _data( data ) {}

    bool operator() ( Triangle tri1, Triangle tri2 )
        {
            const vmml::Vector3f& vertex1_1 = _data->vertices[ tri1[ 0 ]];
            const vmml::Vector3f& vertex2_1 = _data->vertices[ tri2[ 0 ]];
            const vmml::Vector3f& vertex1_2 = _data->vertices[ tri1[ 1 ]];
            const vmml::Vector3f& vertex2_2 = _data->vertices[ tri2[ 1 ]];
            const vmml::Vector3f& vertex1_3 = _data->vertices[ tri1[ 2 ]];
            const vmml::Vector3f& vertex2_3 = _data->vertices[ tri2[ 2 ]];
            
            if( vertex1_1.x != vertex2_1.x )
                return (vertex1_1.x < vertex2_1.x);
            if( vertex1_2.x != vertex2_2.x )
                return (vertex1_2.x < vertex2_2.x);
            if( vertex1_3.x != vertex2_3.x )
                return (vertex1_3.x < vertex2_3.x);
            
            if( vertex1_1.y != vertex2_1.y )
                return (vertex1_1.y < vertex2_1.y);
            if( vertex1_2.y != vertex2_2.y )
                return (vertex1_2.y < vertex2_2.y);
            if( vertex1_3.y != vertex2_3.y )
                return (vertex1_3.y < vertex2_3.y);
            
            if( vertex1_1.z != vertex2_1.z )
                return (vertex1_1.z < vertex2_1.z);
            if( vertex1_2.z != vertex2_2.z )
                return (vertex1_2.z < vertex2_2.z);
            if( vertex1_3.z != vertex2_3.z )
                return (vertex1_3.z < vertex2_3.z);
            
            return false;
        }

    VertexData* _data;
};

struct YAxisSort
{
    YAxisSort( VertexData* data ) : _data( data ) {}

    bool operator() ( Triangle tri1, Triangle tri2 )
        {
            const vmml::Vector3f& vertex1_1 = _data->vertices[ tri1[ 0 ]];
            const vmml::Vector3f& vertex2_1 = _data->vertices[ tri2[ 0 ]];
            const vmml::Vector3f& vertex1_2 = _data->vertices[ tri1[ 1 ]];
            const vmml::Vector3f& vertex2_2 = _data->vertices[ tri2[ 1 ]];
            const vmml::Vector3f& vertex1_3 = _data->vertices[ tri1[ 2 ]];
            const vmml::Vector3f& vertex2_3 = _data->vertices[ tri2[ 2 ]];
            
            if( vertex1_1.y != vertex2_1.y )
                return (vertex1_1.y < vertex2_1.y);
            if( vertex1_2.y != vertex2_2.y )
                return (vertex1_2.y < vertex2_2.y);
            if( vertex1_3.y != vertex2_3.y )
                return (vertex1_3.y < vertex2_3.y);
            
            if( vertex1_1.z != vertex2_1.z )
                return (vertex1_1.z < vertex2_1.z);
            if( vertex1_2.z != vertex2_2.z )
                return (vertex1_2.z < vertex2_2.z);
            if( vertex1_3.z != vertex2_3.z )
                return (vertex1_3.z < vertex2_3.z);
            
            if( vertex1_1.x != vertex2_1.x )
                return (vertex1_1.x < vertex2_1.x);
            if( vertex1_2.x != vertex2_2.x )
                return (vertex1_2.x < vertex2_2.x);
            if( vertex1_3.x != vertex2_3.x )
                return (vertex1_3.x < vertex2_3.x);
            
            return false;
        }

    VertexData* _data;
};

struct ZAxisSort
{
    ZAxisSort( VertexData* data ) : _data( data ) {}

    bool operator() ( Triangle tri1, Triangle tri2 )
        {
            const vmml::Vector3f& vertex1_1 = _data->vertices[ tri1[ 0 ]];
            const vmml::Vector3f& vertex2_1 = _data->vertices[ tri2[ 0 ]];
            const vmml::Vector3f& vertex1_2 = _data->vertices[ tri1[ 1 ]];
            const vmml::Vector3f& vertex2_2 = _data->vertices[ tri2[ 1 ]];
            const vmml::Vector3f& vertex1_3 = _data->vertices[ tri1[ 2 ]];
            const vmml::Vector3f& vertex2_3 = _data->vertices[ tri2[ 2 ]];
            
            if( vertex1_1.z != vertex2_1.z )
                return (vertex1_1.z < vertex2_1.z);
            if( vertex1_2.z != vertex2_2.z )
                return (vertex1_2.z < vertex2_2.z);
            if( vertex1_3.z != vertex2_3.z )
                return (vertex1_3.z < vertex2_3.z);
            
            if( vertex1_1.x != vertex2_1.x )
                return (vertex1_1.x < vertex2_1.x);
            if( vertex1_2.x != vertex2_2.x )
                return (vertex1_2.x < vertex2_2.x);
            if( vertex1_3.x != vertex2_3.x )
                return (vertex1_3.x < vertex2_3.x);
            
            if( vertex1_1.y != vertex2_1.y )
                return (vertex1_1.y < vertex2_1.y);
            if( vertex1_2.y != vertex2_2.y )
                return (vertex1_2.y < vertex2_2.y);
            if( vertex1_3.y != vertex2_3.y )
                return (vertex1_3.y < vertex2_3.y);
            
            return false;
        }

    VertexData* _data;
};

void VertexData::sort( const size_t start, const size_t length, 
                       const SplitAxis axis)
{
    assert( start+length <= triangles.size( ));
    assert( length );

    switch( axis )
    {
        case AXIS_X:
            std::sort( triangles.begin() + start, 
                       triangles.begin() + start + length, XAxisSort( this ));
            break;

        case AXIS_Y:
            std::sort( triangles.begin() + start, 
                       triangles.begin() + start + length, YAxisSort( this ));
            break;

        case AXIS_Z:
            std::sort( triangles.begin() + start, 
                       triangles.begin() + start + length, ZAxisSort( this ));
            break;

        default:
            assert(0);
    }
}
