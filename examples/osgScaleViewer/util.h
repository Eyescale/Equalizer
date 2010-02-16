
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef UTIL_H
#define UTIL_H

#include <osg/Matrix>
#include <eq/client/types.h>
#include <vmmlib/matrix.hpp>

/**
 * Converts a matrix from the VMML library to a matrix of the OSG library.
 * @param matrix a vmml matrix.
 * @return the converted osg matrix.
 */
inline osg::Matrix vmmlToOsg( const eq::Matrix4f& matrix )
{
    return osg::Matrix( matrix( 0, 0 ), matrix( 1, 0 ),
                        matrix( 2, 0 ), matrix( 3, 0 ),
                        matrix( 0, 1 ), matrix( 1, 1 ),
                        matrix( 2, 1 ), matrix( 3, 1 ),
                        matrix( 0, 2 ), matrix( 1, 2 ),
                        matrix( 2, 2 ), matrix( 3, 2 ),
                        matrix( 0, 3 ), matrix( 1, 3 ),
                        matrix( 2, 3 ), matrix( 3, 3 ));
}

/**
 * Computes the orthographics vector.
 * @param vector a vector of 3 float values.
 * @return the corresponding orthographic vector.
 */
inline eq::Vector3f orthographicVector( const eq::Vector3f &vector )
{
    vmml::vector<3,float> result;
    float x = vector.x();
    float z = vector.z();

    result( 0 ) = z;
    result( 2 ) = -x;
    result( 1 ) = 0;

    return result;
}

#if 0
/**
 * Normalizes a vector.
 * @param vector a vector of 3 float values.
 * @return the vector normalized.
 */
inline vmml::vector<3,float> norm( const vmml::vector<3,float> &vector )
{
    float length = sqrt( vector.x() * vector.x() +
                         vector.y() * vector.y() +
                         vector.z() * vector.z( ));

    return ( vector / length );
}
#endif

#endif
