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
/*
jitter.h

This file contains jitter point arrays for 2,3,4,8,15,24 and 66 jitters.
*/

#ifndef JITTER_H_
#define JITTER_H_

#include <eq/base/base.h>
#include <eq/client/types.h>

namespace eq
{
    /* 2 jitter points */
    Vector2f j2[] =
    {
        Vector2f( 0.246490,  0.249999 ),
        Vector2f( -0.246490, -0.249999 )
    };

    /* 3 jitter points */
    Vector2f j3[]=
    {
        Vector2f( -0.373411, -0.250550 ),
        Vector2f( 0.256263,  0.368119 ),
        Vector2f( 0.117148, -0.117570 )
    };

    /* 4 jitter points */
    Vector2f j4[] =
    {
        Vector2f( -0.208147,  0.353730 ),
        Vector2f( 0.203849, -0.353780 ),
        Vector2f( -0.292626, -0.149945 ),
        Vector2f( 0.296924,  0.149994 )
    };

    /* 8 jitter points */
    Vector2f j8[] =
    {
        Vector2f( -0.334818,  0.435331 ),
        Vector2f( 0.286438, -0.393495 ),
        Vector2f( 0.459462,  0.141540 ),
        Vector2f( -0.414498, -0.192829 ),
        Vector2f( -0.183790,  0.082102 ),
        Vector2f( -0.079263, -0.317383 ),
        Vector2f(  0.102254,  0.299133 ),
        Vector2f(  0.164216, -0.054399 )
    };

    /* 15 jitter points */
    Vector2f j15[] =
    {
        Vector2f( 0.285561,  0.188437 ),
        Vector2f( 0.360176, -0.065688 ),
        Vector2f( -0.111751,  0.275019 ),
        Vector2f( -0.055918, -0.215197 ),
        Vector2f( -0.080231, -0.470965 ),
        Vector2f( 0.138721,  0.409168 ),
        Vector2f( 0.384120,  0.458500 ),
        Vector2f( -0.454968,  0.134088 ),
        Vector2f( 0.179271, -0.331196 ),
        Vector2f( -0.307049, -0.364927 ),
        Vector2f( 0.105354, -0.010099 ),
        Vector2f( -0.154180,  0.021794 ),
        Vector2f( -0.370135, -0.116425 ),
        Vector2f(  0.451636, -0.300013 ),
        Vector2f( -0.370610,  0.387504 )
    };

    /* 24 jitter points */
    Vector2f j24[] =
    {
        Vector2f( 0.030245,  0.136384 ),
        Vector2f( 0.018865, -0.348867 ),
        Vector2f( -0.350114, -0.472309 ),
        Vector2f( 0.222181,  0.149524 ),
        Vector2f( -0.393670, -0.266873 ),
        Vector2f( 0.404568,  0.230436 ),
        Vector2f( 0.098381,  0.465337 ),
        Vector2f( 0.462671,  0.442116 ),
        Vector2f( 0.400373, -0.212720 ),
        Vector2f( -0.409988,  0.263345 ),
        Vector2f( -0.115878, -0.001981 ),
        Vector2f( 0.348425, -0.009237 ),
        Vector2f( -0.464016,  0.066467 ),
        Vector2f( -0.138674, -0.468006 ),
        Vector2f( 0.144932, -0.022780 ),
        Vector2f( -0.250195,  0.150161 ),
        Vector2f( -0.181400, -0.264219 ),
        Vector2f(  0.196097, -0.234139 ),
        Vector2f( -0.311082, -0.078815 ),
        Vector2f( 0.268379,  0.366778 ),
        Vector2f( -0.040601,  0.327109 ),
        Vector2f( -0.234392,  0.354659 ),
        Vector2f( -0.003102, -0.154402 ),
        Vector2f( 0.297997, -0.417965 )
    };

    /* 66 jitter points */
    Vector2f j66[] =
    {
        Vector2f( 0.266377, -0.218171 ),
        Vector2f( -0.170919, -0.429368 ),
        Vector2f( 0.047356, -0.387135 ),
        Vector2f( -0.430063,  0.363413 ),
        Vector2f( -0.221638, -0.313768 ),
        Vector2f(  0.124758, -0.197109 ),
        Vector2f( -0.400021,  0.482195 ),
        Vector2f( 0.247882,  0.152010 ),
        Vector2f( -0.286709, -0.470214 ),
        Vector2f( -0.426790,  0.004977 ),
        Vector2f( -0.361249, -0.104549 ),
        Vector2f( -0.040643,  0.123453 ),
        Vector2f( -0.189296,  0.438963 ),
        Vector2f( -0.453521, -0.299889 ),
        Vector2f( 0.408216, -0.457699 ),
        Vector2f( 0.328973, -0.101914 ),
        Vector2f( -0.055540, -0.477952 ),
        Vector2f( 0.194421,  0.453510 ),
        Vector2f( 0.404051,  0.224974 ),
        Vector2f( 0.310136,  0.419700 ),
        Vector2f( -0.021743,  0.403898 ),
        Vector2f( -0.466210,  0.248839 ),
        Vector2f( 0.341369,  0.081490 ),
        Vector2f( 0.124156, -0.016859 ),
        Vector2f( -0.461321, -0.176661 ),
        Vector2f( 0.013210,  0.234401 ),
        Vector2f( 0.174258, -0.311854 ),
        Vector2f( 0.294061,  0.263364 ),
        Vector2f( -0.114836,  0.328189 ),
        Vector2f( 0.041206, -0.106205 ),
        Vector2f( 0.079227,  0.345021 ),
        Vector2f( -0.109319, -0.242380 ),
        Vector2f( 0.425005, -0.332397 ),
        Vector2f( 0.009146,  0.015098 ),
        Vector2f( -0.339084, -0.355707 ),
        Vector2f( -0.224596, -0.189548 ),
        Vector2f( 0.083475,  0.117028 ),
        Vector2f( 0.295962, -0.334699 ),
        Vector2f( 0.452998,  0.025397 ),
        Vector2f( 0.206511, -0.104668 ),
        Vector2f( 0.447544, -0.096004 ),
        Vector2f( -0.108006, -0.002471 ),
        Vector2f( -0.380810,  0.130036 ),
        Vector2f( -0.242440,  0.186934 ),
        Vector2f( -0.200363,  0.070863 ),
        Vector2f( -0.344844, -0.230814 ),
        Vector2f( 0.408660,  0.345826 ),
        Vector2f( -0.233016,  0.305203 ),
        Vector2f( 0.158475, -0.430762 ),
        Vector2f( 0.486972,  0.139163 ),
        Vector2f( -0.301610,  0.009319 ),
        Vector2f( 0.282245, -0.458671 ),
        Vector2f( 0.482046,  0.443890 ),
        Vector2f( -0.121527,  0.210223 ),
        Vector2f( -0.477606, -0.424878 ),
        Vector2f( -0.083941, -0.121440 ),
        Vector2f( -0.345773,  0.253779 ),
        Vector2f( 0.234646,  0.034549 ),
        Vector2f( 0.394102, -0.210901 ),
        Vector2f( -0.312571,  0.397656 ),
        Vector2f( 0.200906,  0.333293 ),
        Vector2f( 0.018703, -0.261792 ),
        Vector2f( -0.209349, -0.065383 ),
        Vector2f( 0.076248,  0.478538 ),
        Vector2f( -0.073036, -0.355064 ),
        Vector2f( 0.145087,  0.221726 )
    };
}

#endif /* JITTER_H_ */
