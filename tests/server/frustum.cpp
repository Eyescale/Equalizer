
/* Copyright (c) 2013-2016, Stefan Eilemann <eilemann@equalizergraphics.com>
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
#include <eq/server/frustumData.h>

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

using namespace eq::server;

int main( int, char** )
{
    FrustumData data;
    const Wall wall( Vector3f( -.8f, -.5f, -1.f ),
                     Vector3f(  .8f, -.5f, -1.f ),
                     Vector3f( -.8f,  .5f, -1.f ));
    data.applyWall( wall );

    Matrix4f xfm;
    xfm.array[14] = 1.f;
    TESTINFO( data.getTransform() == xfm, data.getTransform( ));
    TESTINFO( data.getWidth() == 1.6f, data.getWidth( ));
    TESTINFO( data.getHeight() == 1.0f, data.getHeight( ));


    const Wall left( Vector3f( -1, -1,  1 ),
                     Vector3f( -1, -1, -1 ),
                     Vector3f( -1,  1,  1 ));
    data.applyWall( left );

    xfm.array[14] = 0.f;
    xfm.rotate_y( -M_PI_2 );
    xfm.array[14] = 1.f;
    TESTINFO( xfm.equals( data.getTransform(), 0.0001f ),
              std::endl << data.getTransform() << xfm );
    TESTINFO( data.getWidth() == 2.f, data.getWidth( ));
    TESTINFO( data.getHeight() == 2.f, data.getHeight( ));


    const Wall offaxis( Vector3f( -1,  0,  1 ),
                        Vector3f( -1,  0,  0 ),
                        Vector3f( -1,  1,  1 ));
    data.applyWall( offaxis );

    xfm.array[12] =  .5f;
    xfm.array[13] = -.5f;
    TESTINFO( xfm.equals( data.getTransform(), 0.0001f ),
              std::endl << data.getTransform() << xfm );
    TESTINFO( data.getWidth() == 1.f, data.getWidth( ));
    TESTINFO( data.getHeight() == 1.f, data.getHeight( ));

    return EXIT_SUCCESS;
}
