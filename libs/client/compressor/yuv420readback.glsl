/* Copyright (c) 2009       Maxim Makhinya
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

uniform sampler2DRect color;

vec4 yMul = vec4( 0.299 , 0.587 , 0.114 , 0.0 );
vec4 uMul = vec4(-0.147 ,-0.289 , 0.436 , 0.0 );
vec4 vMul = vec4( 0.4996,-0.4184,-0.0812, 0.0 );
/*vec4 vMul = vec4( 0.2896,-0.2084,-0.0802, 0.0 );*/

vec4 test = vec4( 1.0, 1.0, 1.0, 0.0 );

void main(void)
{
    vec2 pos = gl_FragCoord.xy - vec2( 0.5, 0.5 );
    pos.x = pos.x * 2.0;

    float odd = fract( pos.y*0.5 );
    float hor;
    if( odd < 0.5 )
        hor =  1.0;
    else
        hor = -1.0;

    vec4 p1 = texture2DRect( color, pos );
    vec4 p2 = texture2DRect( color, pos + vec2( 1.0, 0.0 ));
    vec4 p3 = texture2DRect( color, pos + vec2( 0.0, hor ));
    vec4 p4 = texture2DRect( color, pos + vec2( 1.0, hor ));

    float n = 0.0;
    vec4  p = vec4( .0, .0, .0, .0 );

    if( dot( p1, test ) > .0 ) { n += 1.0; p += p1; }
    if( dot( p2, test ) > .0 ) { n += 1.0; p += p2; }
    if( dot( p3, test ) > .0 ) { n += 1.0; p += p3; }
    if( dot( p4, test ) > .0 ) { n += 1.0; p += p4; }

    float cVal = 0.0;
    if( n > 0.0 )
    {
        p /= n;

        if( odd < 0.5 )
            cVal = dot(p,uMul);
        else
            cVal = dot(p,vMul);
    }
//    cVal = 0.0;
    gl_FragColor = vec4( dot(p1,yMul), dot(p2,yMul), cVal+0.5, (p1.a+p2.a)/2.0 );
}

