
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <lunchbox/test.h>

#include <eq/fabric/wall.h>

using namespace eq::fabric;

// Tests the wall description

int main( int, char** )
{
    const Wall wall( Vector3f( -.5f, -.5f, -.5f ),
                     Vector3f( 1.f,  1.f,  -.5f ),
                     Vector3f( -.5f, -.5f,  .5f ));
    Wall target( Vector3f( -.875f,  -.875f, -.5f ),
                 Vector3f( 1.375f,  1.375f, -.5f ),
                 Vector3f( -.875f,  -.875f,  .5f ));
    Wall tmp( wall );
    tmp.resizeHorizontal( 1.5f );
    TESTINFO( tmp == target, tmp << " != " << target );

    target.bottomLeft  = Vector3f( -.125f, -.125f, -.5f );
    target.bottomRight = Vector3f(  .625f,  .625f, -.5f );
    target.topLeft     = Vector3f( -.125f, -.125f,  .5f );

    tmp = wall;
    tmp.resizeHorizontal( .5f );
    TESTINFO( tmp == target, tmp << " != " << target );


    target.bottomLeft  = Vector3f( -.5f, -.5f, -.25f );
    target.bottomRight = Vector3f(  1.f, 1.f,  -.25f );
    target.topLeft     = Vector3f( -.5f, -.5f,  .25f );

    tmp = wall;
    tmp.resizeVertical( .5f );
    TESTINFO( tmp == target, tmp << " != " << target );


    const Frustumf frustum( -.8f, .8f, -.5f, .5f, 1.f, 100.f );
    const Wall wall2( Vector3f( -.8f, -.5f, -1.f ),
                      Vector3f(  .8f, -.5f, -1.f ),
                      Vector3f( -.8f,  .5f, -1.f ));

    target = frustum.computePerspectiveMatrix().inverse();
    TESTINFO( wall2 == target, wall2 << " != " << target );

    return EXIT_SUCCESS;
}
