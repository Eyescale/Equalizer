/* Copyright (c) 2009       Maxim Makhinya
   All rights reserved. */

uniform sampler2DRect depth;

void main(void)
{

    float min = 1.0;
    float max = 0.0;
    float bg  = 0.0;

    vec2  pos = gl_TexCoord[0].st;
    pos *= vec2(16,16);

    float xs = 16.0;
    float ys = 16.0;

    for( float y = .0; y < 16.0; y+=1.0 )
    {
        for( float x = .0; x < 16.0; x+=1.0 )
        {
            float s = texture2DRect( depth, pos + vec2( x, y )).x;

            if( s == 1.0 )
                bg += (1.0/256.0);
            else
            {
                min = s;
                max = s;
                xs  = x + 1.0;
                ys  = y;
                x   = 16.0;
                y   = 16.0;
            }
        }
    }

    for( float y = ys; y < 16.0; y+=1.0 )
    {
        for( float x = xs; x < 16.0; x+=1.0 )
        {
            float s = texture2DRect( depth, pos + vec2( x, y )).x;

            if( s != 1.0 )
            {
                if( s > max )
                    max = s;
                else
                    if( s < min )
                        min = s;
            }else
                bg += (1.0/256.0);
        }
        xs = .0;
    }

    gl_FragColor.r = min;
    gl_FragColor.g = max;
    gl_FragColor.b = bg;
    gl_FragColor.a = 0.0;
}
