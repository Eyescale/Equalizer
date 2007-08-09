
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include "vertexData.h"

#include <iterator>

#include <eq/base/rng.h>

using namespace mesh;
using namespace std;

// Tests the functionality of the triangle mesh library

int main( int argc, char **argv )
{
    VertexData data;
    
    for( unsigned i=0; i<10000; ++i )
        data.vertices.push_back( vmml::Vector3f( ));

    const size_t nIndices = data.vertices.size() - 3;
    for( size_t i = 0; i < nIndices; i += 3 )
        data.triangles.push_back( vmml::Vector3< size_t >( i, i+1, i+2 ));

    data.sort( 0, data.triangles.size(), AXIS_Y );

    for( vector< Triangle >::const_iterator i = data.triangles.begin() + 1;
         i < data.triangles.end(); ++i )
    {
        const Triangle& tri1 = *(i-1);
        const Triangle& tri2 = *i;
        
        const vmml::Vector3f& vertex1 = data.vertices[ tri1[0] ];
        const vmml::Vector3f& vertex2 = data.vertices[ tri2[0] ];
        TEST( vertex1.y <= vertex2.y );
    }   
}
