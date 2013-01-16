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


uniform float shiftX;
uniform float shiftY;

void main(void)
{
    vec2 pos = gl_FragCoord.xy - vec2( 0.5 + shiftX, 0.5 + shiftY );

    vec2 posAbs = vec2( floor( pos.x * 0.5 ), pos.y );

    float odd = fract( pos.y*0.5 );
    float hor;
    if( odd < 0.5 )
        hor =  1.0;
    else
        hor = -1.0;

    vec4 p1 = texture2DRect( color, posAbs );
    vec4 p2 = texture2DRect( color, posAbs + vec2( 0.0, hor ));

    float u;
    float v;
    if( odd < 0.5 )
    {
        u = p1.b - 0.5;
        v = p2.b - 0.5;
    }else
    {
        u = p2.b - 0.5;
        v = p1.b - 0.5;
    }

    //u = 0.0; v = 0.0;

    float y;
    if( fract( pos.x*0.5 ) < 0.5 )
        y = p1.x;
    else
        y = p1.y;

    gl_FragColor.r = y              + 1.4031 * v;
    gl_FragColor.g = y - 0.3945 * u - 0.7146 * v;
    gl_FragColor.b = y + 2.032  * u;
    gl_FragColor.a = p1.a;
}

