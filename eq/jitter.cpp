/*
 * (c) Copyright 1993, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */


#include "jitter.h"
#include <eq/fabric/vmmlib.h>

namespace eq
{
Vector2f Jitter::j2[2] = { Vector2f( 0.246490f,  0.249999f ),
                           Vector2f( -0.246490f, -0.249999f ) };

Vector2f Jitter::j3[3] = { Vector2f( -0.373411f, -0.250550f ),
                           Vector2f( 0.256263f,  0.368119f ),
                           Vector2f( 0.117148f, -0.117570f ) };

Vector2f Jitter::j4[4] = { Vector2f( -0.208147f,  0.353730f ),
                           Vector2f( 0.203849f, -0.353780f ),
                           Vector2f( -0.292626f, -0.149945f ),
                           Vector2f( 0.296924f,  0.149994f ) };

Vector2f Jitter::j8[8] = { Vector2f( -0.334818f,  0.435331f ),
                           Vector2f( 0.286438f, -0.393495f ),
                           Vector2f( 0.459462f,  0.141540f ),
                           Vector2f( -0.414498f, -0.192829f ),
                           Vector2f( -0.183790f,  0.082102f ),
                           Vector2f( -0.079263f, -0.317383f ),
                           Vector2f(  0.102254f,  0.299133f ),
                           Vector2f(  0.164216f, -0.054399f )};

Vector2f Jitter::j15[15] = { Vector2f( 0.285561f,  0.188437f ),
                             Vector2f( 0.360176f, -0.065688f ),
                             Vector2f( -0.111751f,  0.275019f),
                             Vector2f( -0.055918f, -0.215197f ),
                             Vector2f( -0.080231f, -0.470965f ),
                             Vector2f( 0.138721f,  0.409168f ),
                             Vector2f( 0.384120f,  0.458500f ),
                             Vector2f( -0.454968f,  0.134088f ),
                             Vector2f( 0.179271f, -0.331196f ),
                             Vector2f( -0.307049f, -0.364927f ),
                             Vector2f( 0.105354f, -0.010099f ),
                             Vector2f( -0.154180f,  0.021794f ),
                             Vector2f( -0.370135f, -0.116425f ),
                             Vector2f(  0.451636f, -0.300013f ),
                             Vector2f( -0.370610f,  0.387504f ) };

Vector2f Jitter::j24[24] = { Vector2f( 0.030245f,  0.136384f ),
                             Vector2f( 0.018865f, -0.348867f ),
                             Vector2f( -0.350114f, -0.472309f ),
                             Vector2f( 0.222181f,  0.149524f ),
                             Vector2f( -0.393670f, -0.266873f ),
                             Vector2f( 0.404568f,  0.230436f ),
                             Vector2f( 0.098381f,  0.465337f ),
                             Vector2f( 0.462671f,  0.442116f ),
                             Vector2f( 0.400373f, -0.212720f ),
                             Vector2f( -0.409988f,  0.263345f ),
                             Vector2f( -0.115878f, -0.001981f ),
                             Vector2f( 0.348425f, -0.009237f ),
                             Vector2f( -0.464016f,  0.066467f ),
                             Vector2f( -0.138674f, -0.468006f ),
                             Vector2f( 0.144932f, -0.022780f ),
                             Vector2f( -0.250195f,  0.150161f ),
                             Vector2f( -0.181400f, -0.264219f ),
                             Vector2f(  0.196097f, -0.234139f ),
                             Vector2f( -0.311082f, -0.078815f ),
                             Vector2f( 0.268379f,  0.366778f ),
                             Vector2f( -0.040601f,  0.327109f ),
                             Vector2f( -0.234392f,  0.354659f ),
                             Vector2f( -0.003102f, -0.154402f ),
                             Vector2f( 0.297997f, -0.417965f ) };

Vector2f Jitter::j66[66] = { Vector2f( 0.266377f, -0.218171f ),
                             Vector2f( -0.170919f, -0.429368f ),
                             Vector2f( 0.047356f, -0.387135f ),
                             Vector2f( -0.430063f,  0.363413f ),
                             Vector2f( -0.221638f, -0.313768f ),
                             Vector2f(  0.124758f, -0.197109f ),
                             Vector2f( -0.400021f,  0.482195f ),
                             Vector2f( 0.247882f,  0.152010f ),
                             Vector2f( -0.286709f, -0.470214f ),
                             Vector2f( -0.426790f,  0.004977f ),
                             Vector2f( -0.361249f, -0.104549f ),
                             Vector2f( -0.040643f,  0.123453f ),
                             Vector2f( -0.189296f,  0.438963f ),
                             Vector2f( -0.453521f, -0.299889f ),
                             Vector2f( 0.408216f, -0.457699f ),
                             Vector2f( 0.328973f, -0.101914f ),
                             Vector2f( -0.055540f, -0.477952f ),
                             Vector2f( 0.194421f,  0.453510f ),
                             Vector2f( 0.404051f,  0.224974f ),
                             Vector2f( 0.310136f,  0.419700f ),
                             Vector2f( -0.021743f,  0.403898f ),
                             Vector2f( -0.466210f,  0.248839f ),
                             Vector2f( 0.341369f,  0.081490f ),
                             Vector2f( 0.124156f, -0.016859f ),
                             Vector2f( -0.461321f, -0.176661f ),
                             Vector2f( 0.013210f,  0.234401f ),
                             Vector2f( 0.174258f, -0.311854f ),
                             Vector2f( 0.294061f,  0.263364f ),
                             Vector2f( -0.114836f,  0.328189f ),
                             Vector2f( 0.041206f, -0.106205f ),
                             Vector2f( 0.079227f,  0.345021f ),
                             Vector2f( -0.109319f, -0.242380f ),
                             Vector2f( 0.425005f, -0.332397f ),
                             Vector2f( 0.009146f,  0.015098f ),
                             Vector2f( -0.339084f, -0.355707f ),
                             Vector2f( -0.224596f, -0.189548f ),
                             Vector2f( 0.083475f,  0.117028f ),
                             Vector2f( 0.295962f, -0.334699f ),
                             Vector2f( 0.452998f,  0.025397f ),
                             Vector2f( 0.206511f, -0.104668f ),
                             Vector2f( 0.447544f, -0.096004f ),
                             Vector2f( -0.108006f, -0.002471f ),
                             Vector2f( -0.380810f,  0.130036f ),
                             Vector2f( -0.242440f,  0.186934f ),
                             Vector2f( -0.200363f,  0.070863f ),
                             Vector2f( -0.344844f, -0.230814f ),
                             Vector2f( 0.408660f,  0.345826f ),
                             Vector2f( -0.233016f,  0.305203f ),
                             Vector2f( 0.158475f, -0.430762f ),
                             Vector2f( 0.486972f,  0.139163f ),
                             Vector2f( -0.301610f,  0.009319f ),
                             Vector2f( 0.282245f, -0.458671f ),
                             Vector2f( 0.482046f,  0.443890f ),
                             Vector2f( -0.121527f,  0.210223f ),
                             Vector2f( -0.477606f, -0.424878f ),
                             Vector2f( -0.083941f, -0.121440f ),
                             Vector2f( -0.345773f,  0.253779f ),
                             Vector2f( 0.234646f,  0.034549f ),
                             Vector2f( 0.394102f, -0.210901f ),
                             Vector2f( -0.312571f,  0.397656f ),
                             Vector2f( 0.200906f,  0.333293f ),
                             Vector2f( 0.018703f, -0.261792f ),
                             Vector2f( -0.209349f, -0.065383f ),
                             Vector2f( 0.076248f,  0.478538f ),
                             Vector2f( -0.073036f, -0.355064f ),
                             Vector2f( 0.145087f,  0.221726f ) };
}
