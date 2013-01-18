/* Copyright (c) 2009       Maxim Makhinya
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

uniform sampler2DRect texture;

void main(void)
{
    vec2  pos = (gl_FragCoord.xy - vec2( 0.5, 0.5 ))*16.0;

    float bg  = 1.0;

    float s;
    for( float y = .0; y < 15.1; y+=2.0 )
    {
        if( bg < .5 )
            break;
        for( float x = .0; x < 15.1; x+=2.0 )
        {
            s = texture2DRect( texture, pos + vec2( x, y )).x;
            if( s != 1.0 )
            {
                bg = .0;
                break;
            }
        }
    }
    for( float y = 1.0; y < 15.1; y+=2.0 )
    {
        if( bg < .5 )
            break;
        for( float x = 1.0; x < 15.1; x+=2.0 )
        {
            s = texture2DRect( texture, pos + vec2( x, y )).x;

            if( s != 1.0 )
            {
                bg = .0;
                break;
            }
        }
    }
    for( float y = .0; y < 15.1; y+=2.0 )
    {
        if( bg < .5 )
            break;
        for( float x = 1.0; x < 15.1; x+=2.0 )
        {
            s = texture2DRect( texture, pos + vec2( x, y )).x;

            if( s != 1.0 )
            {
                bg = .0;
                break;
            }
        }
    }
    for( float y = 1.0; y < 15.1; y+=2.0 )
    {
        if( bg < .5 )
            break;
        for( float x = .0; x < 15.1; x+=2.0 )
        {
            s = texture2DRect( texture, pos + vec2( x, y )).x;

            if( s != 1.0 )
            {
                bg = .0;
                break;
            }
        }
    }

    gl_FragColor = vec4( bg, .0, .0, .0 );
}

