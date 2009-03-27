
/* Copyright (c) 2009, Cedric Stalder
                 2009, Stefan Eilemann <eilemann@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <test.h>
#include <eq/client/wall.h>
#include <eq/client/projection.h>

using namespace eq;
using namespace std;

int main( int argc, char **argv )
{
    // Test for same default values
    Projection projection;
    Projection projection2;
    Wall wall;
    projection2 = wall;

    TESTINFO( projection == projection2,
              projection << std::endl << projection2 );

    // Test 1 
    projection.distance = 3;
    projection.fov[0] = 90;
    projection.fov[1] = 90;
    projection.origin[0] = 4;
    projection.origin[1] = 0;
    projection.origin[2] = 0;

    
    wall = projection;

    projection2 = wall;
    
    TESTINFO( projection == projection2 , 
              projection << std::endl << wall << std::endl <<
              projection2  << std::endl );

    // test 2
    projection.distance = 3;
    projection.fov[0] = 90;
    projection.fov[1] = 90;
    projection.origin[0] = -4;
    projection.origin[1] = -3;
    projection.origin[2] = -2;
    wall = projection;
    projection2 = wall;
    TESTINFO( projection == projection2 , "Test 2" <<
             projection << std::endl << wall << std::endl <<
             projection2  << std::endl ); 
    
    
    // test 3
    projection.distance = 3;
    projection.fov[0] = 90;
    projection.fov[1] = 90;
    projection.origin[0] = 4;
    projection.origin[1] = 3;
    projection.origin[2] = 2;
    wall = projection;
    projection2 = wall;
    TESTINFO( projection == projection2 , "Test 3" <<
             projection << std::endl << wall << std::endl <<
             projection2  << std::endl ); 
             
             
    // test 4
    projection.distance = 3;
    projection.fov[0] = 43;
    projection.fov[1] = 20;
    projection.origin[0] = 4;
    projection.origin[1] = 3;
    projection.origin[2] = 2;
    
    projection.hpr[0] = 90;
    projection.hpr[1] = -68;
    projection.hpr[2] = -77;
    
    wall = projection;
    projection2 = wall;
    Wall wall2;
    wall2=  projection2; 
    TESTINFO( projection == projection2 , "Test 4" <<
             projection << std::endl << wall << std::endl <<
             projection2 << std::endl << wall2 << std::endl ); 

#if 0
    // Test n
    wall.bottomLeft  = vmml::Vector3f( 2.4749f, -9.4749f, -5.5303f );
    wall.bottomRight = vmml::Vector3f( 4.5251f, 2.4749f, -1.4497f );
    wall.topLeft     = vmml::Vector3f( -4.5251f, -2.4749, 15.45f );

    projection = wall;
    wall2 = projection; 
    TESTINFO( wall == wall2, wall << projection << wall2 );
#endif
}
