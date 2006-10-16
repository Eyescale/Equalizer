/* Copyright (c) 2006, Dustin Wüest <wueest@dustin.ch>
   All rights reserved.
   Adapted code for Equalizer usage.
 */

/******************************************************************************
 *                                                                            *
 * Copyright (c) 2002 Silicon Graphics, Inc.  All Rights Reserved.            *
 *                                                                            *
 * The recipient ("Recipient") of this software, including as modified        *
 * ("Software") may reproduce, redistribute, use, and derive works from the   *
 * Software without restriction, subject to the following conditions:         *
 *                                                                            *
 * - Redistribution of the Software in any form must reproduce this entire    *
 *   notice, including as modified in accordance with these provisions        *
 *   ("Notice");                                                              *
 * - Any Recipient who modifies and subsequently redistributes the Software   *
 *   shall add information to this Notice to sufficiently identify the        *
 *   Recipient's modifications;                                               *
 * - Recipient may not use the name(s) of any previous Recipient to endorse   *
 *   or promote any products derived from the Software without prior express  *
 *   written permission from such previous Recipient.                         *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT ANY EXPRESS OR IMPLIED WARRANTY  *
 * OR CONDITION, INCLUDING WITHOUT LIMITATION ANY WARRANTIES OR CONDITIONS OF *
 * MERCHANTABILITY, SECURITY, SATISFACTORY QUALITY, FITNESS FOR A PARTICULAR  *
 * PURPOSE, AND NON-INFRINGEMENT.  PATENT LICENSES, IF ANY, PROVIDED HEREIN   *
 * DO NOT APPLY TO COMBINATIONS OF THIS PROGRAM WITH OTHER SOFTWARE, OR ANY   *
 * OTHER PRODUCT WHATSOEVER.  IN NO EVENT WILL THE ORIGINATOR OR SUBSEQUENT   *
 * RECIPIENT OF THE SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING IN RELATION TO THE    *
 * SOFTWARE, ITS USE, OR THESE PROVISIONS, HOWEVER CAUSED AND ON ANY THEORY   *
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR NEGLIGENCE OR      *
 * OTHER TORT, EVEN IF PRE-ADVISED OF THE PROSPECT OF SUCH DAMAGES.           *
 *                                                                            *
 * This Software was developed at private expense; if acquired under an       *
 * agreement with the USA government or any contractor thereto, it is         *
 * acquired as "commercial computer software" subject to the provisions of    *
 * this license agreement, as specified in (a) 48 CFR 12.212 of the FAR; or,  *
 * if acquired for Department of Defense units, (b) 48 CFR 227-7202 of the    *
 * DoD FAR Supplement; or sections succeeding thereto.                        *
 *                                                                            *
 *                                                                            *
 * Originator:  Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,               *
 * Mountain View, CA  94043.  http://www.sgi.com                              *
 *                                                                            *
 ******************************************************************************/

#ifndef _FRUSTUM_H_
#define	_FRUSTUM_H_

#include "vertex.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// * * * * * * * * * *
//
// - declaration -
//
// * * * * * * * * * *

enum FrustumVisibility
{
    FRUSTUM_VISIBILITY_FULL,
    FRUSTUM_VISIBILITY_PARTIAL,
    FRUSTUM_VISIBILITY_NULL
};


template< typename T > 
class Frustum
{
public:

    Frustum(){};
    ~Frustum(){};

    void initView( void );
    void initView( T proj[16], T model[16], const eq::PixelViewport& pvp );

    // frustum culling tests
    bool pointInFrustum( const float x, const float y, const float z );
    FrustumVisibility sphereVisibility( const float x, const float y, 
        const float z, const float r, float *distance = NULL );
    FrustumVisibility sphereVisibility( const float pos[3], const float r, 
        float *distance = NULL );

    // visibility in screen space
    bool sphereScreenRegion( const float x, const float y, const float z,
        const float r,                                           // in
        float vp[4], int pvp[4]=NULL );                          // out
    bool sphereScreenRegion( const float pos[3], const float r,  // in
        float vp[4], int pvp[4]=NULL );                          // out

    bool boxScreenRegion( const float box[2][3],                 // in
        float vp[4], int pvp[4]=NULL );                          // out
    bool boxScreenRegion( const Vertex box[2],                   // in
        float vp[4], int pvp[4]=NULL );                          // out

private:
    T _proj[16];
    T _model[16];

    T      _frustum[6][4];
    double _screenUp[3];
    double _screenRight[3];
    eq::PixelViewport _pvp;

    struct {
        bool screenVec;
    } _dirty;

    void initScreenVec( void );
    void normalize( double vector[3] );
};

typedef Frustum<float>  Frustumf;

// * * * * * * * * * *
//
// - implementation -
//
// * * * * * * * * * *

//---------------------------------------------------------------------------
// pointInFrustum
//---------------------------------------------------------------------------
#if 0
template< typename T > 
bool Frustum< T >::pointInFrustum( const float x, const float y, const float z )
{
    for( int i = 0; i < 6; i++ )
      if( _frustum[i][0] * x + _frustum[i][1] * y + 
          _frustum[i][2] * z + _frustum[i][3] <= 0. )
         return false;

   return true;
}
#endif
//---------------------------------------------------------------------------
// sphereVisiblity
//---------------------------------------------------------------------------
template< typename T > 
FrustumVisibility Frustum< T >::sphereVisibility( const float pos[3], 
                                                  const float r, 
                                                  float *distance )
{
    return sphereVisibility( pos[0], pos[1], pos[2], r, distance );
}

template< typename T > 
FrustumVisibility Frustum< T >::sphereVisibility( const float x, const float y,
    const float z, const float r, float *distance )
{
    float minDistance = r;
    float d;

    d = _frustum[0][0] * x + _frustum[0][1] * y + _frustum[0][2] * z +
        _frustum[0][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;

    d = _frustum[1][0] * x + _frustum[1][1] * y + _frustum[1][2] * z +
        _frustum[1][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;

    d = _frustum[2][0] * x + _frustum[2][1] * y + _frustum[2][2] * z +
        _frustum[2][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;

    d = _frustum[3][0] * x + _frustum[3][1] * y + _frustum[3][2] * z +
        _frustum[3][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;

    d = _frustum[4][0] * x + _frustum[4][1] * y + _frustum[4][2] * z +
        _frustum[4][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;

    d = _frustum[5][0] * x + _frustum[5][1] * y + _frustum[5][2] * z +
        _frustum[5][3];
    if( d <= -r )
        return FRUSTUM_VISIBILITY_NULL;
    minDistance = (d < minDistance) ? d : minDistance;


    if( distance != NULL )
        *distance = d;

    if( minDistance == r )
        return FRUSTUM_VISIBILITY_FULL;

    return FRUSTUM_VISIBILITY_PARTIAL;
}

//---------------------------------------------------------------------------
// sphereScreenRegion
//---------------------------------------------------------------------------

// Note: This method does not provide the exact result for perspective frusta.
//       It does compute the region based on the top-most and right point,
//       when using a perspective frustum these are not the outmost points of
//       the sphere.
#if 0
template< typename T > 
bool Frustum< T >::sphereScreenRegion( const float pos[3], const float r,
    float vp[4], int pvp[4] )
{
   return sphereScreenRegion( pos[0], pos[1], pos[2], r, vp, pvp );
}

template< typename T > 
bool Frustum< T >::sphereScreenRegion( const float x, const float y, const float z,
    const float r, float vp[4], int pvp[4] )
{
    const float sphereDistance = _frustum[5][0] * x + _frustum[5][1] * y + 
                             _frustum[5][2] * z + _frustum[5][3]+_frustum[5][3];
    const float frontDistance  = _frustum[5][3];

    if( sphereDistance+r < frontDistance )
        return false;

    GLdouble top[3], right[3];
    int view[4] = { 0, 0, 1, 1 };

    initScreenVec();

    // upper point
    gluProject( x + r*_screenUp[0], y + r*_screenUp[1], z + r*_screenUp[2],
        _model, _proj, view,
        &top[0], &top[1], &top[2] );
    
    // right point
    gluProject( x+r*_screenRight[0], y+r*_screenRight[1], z+r*_screenRight[2],
        _model, _proj, view,
        &right[0], &right[1], &right[2] );
    
    // fractional viewport
    vp[2] = 2*( right[0] - top[0] );
    vp[3] = 2*( top[1]   - right[1] );
    vp[0] = right[0] - vp[2];
    vp[1] = top[1]   - vp[3];

    // pixel viewport
    if( pvp != NULL )
    {
        pvp[0] = (int)(vp[0] * _pvp.w + _pvp.x );
        pvp[1] = (int)(vp[1] * _pvp.h + _pvp.y);
        pvp[2] = (int)(vp[2] * _pvp.w);
        pvp[3] = (int)(vp[3] * _pvp.h);
    }

    return true;
}

//---------------------------------------------------------------------------
// boxScreenRegion
//---------------------------------------------------------------------------
template< typename T > 
bool Frustum< T >::boxScreenRegion( const Vertex box[2], float vp[4], int pvp[4] )
{
    float fbox[2][3];
    
    fbox[0][0] = box[0].pos[0];
    fbox[0][1] = box[0].pos[1];
    fbox[0][2] = box[0].pos[2];
    fbox[1][0] = box[1].pos[0];
    fbox[1][1] = box[1].pos[1];
    fbox[1][2] = box[1].pos[2];

    return boxScreenRegion( fbox, vp, pvp );
}

template< typename T > 
bool Frustum< T >::boxScreenRegion( const float box[2][3], float vp[4], int pvp[4] )
{
    // world coordinate of the box vertices
    float worldCoordinate [8][3];

    worldCoordinate[0][0] = box[0][0];
    worldCoordinate[0][1] = box[0][1];
    worldCoordinate[0][2] = box[0][2];

    worldCoordinate[1][0] = box[1][0];
    worldCoordinate[1][1] = box[0][1];
    worldCoordinate[1][2] = box[0][2];

    worldCoordinate[2][0] = box[0][0];
    worldCoordinate[2][1] = box[1][1];
    worldCoordinate[2][2] = box[0][2];

    worldCoordinate[3][0] = box[1][0];
    worldCoordinate[3][1] = box[1][1];
    worldCoordinate[3][2] = box[0][2];

    worldCoordinate[4][0] = box[0][0];
    worldCoordinate[4][1] = box[0][1];
    worldCoordinate[4][2] = box[1][2];

    worldCoordinate[5][0] = box[1][0];
    worldCoordinate[5][1] = box[0][1];
    worldCoordinate[5][2] = box[1][2];

    worldCoordinate[6][0] = box[0][0];
    worldCoordinate[6][1] = box[1][1];
    worldCoordinate[6][2] = box[1][2];

    worldCoordinate[7][0] = box[1][0];
    worldCoordinate[7][1] = box[1][1];
    worldCoordinate[7][2] = box[1][2];

    GLint    view[4] = { 0, 0, 1, 1 };  
    GLdouble winx, winy, winz; // window coordinates returned by gluProject
    GLdouble minX, minY, maxX, maxY, minZ, maxZ; // bounding box coordinates
   
    // initialize the min/max values
    minX =  FLT_MAX; minY =  FLT_MAX; minZ =  FLT_MAX;
    maxX = -FLT_MAX; maxY = -FLT_MAX; maxZ = -FLT_MAX;
    
    // project the box vertices to window coordinates
    for ( int i=0; i<8 ; i++ )
    {
        if ( !gluProject( worldCoordinate[i][0],
                 worldCoordinate[i][1],
                 worldCoordinate[i][2],
                 _model, _proj, view,
                 &winx, &winy, &winz) )
        {
            EQWARN << "gluProject returned GL_FALSE" << endl;
            continue;
        }
        
        // update the min/max bounding box coordinates
        if ( winx < minX ) minX = winx;
        if ( winy < minY ) minY = winy;
        if ( winz < minZ ) minZ = winz;

        if ( winx > maxX ) maxX = winx;
        if ( winy > maxY ) maxY = winy;
        if ( winz > maxZ ) maxZ = winz;
    }
    
    // Now we have the bounding box coordinates.
    
    // Test the special case where the object is outside the viewport
    if (( minZ > 1. ) ||  // outside against us 
        ( maxZ < 0. ) ||  // before the near plane 
        ( minX > 1. ) ||  // on the right
        ( minY > 1. ) ||  // on top
        ( maxX < 0. ) ||  // on the left
        ( maxY < 0. ))    // below
    {
        vp[0] = 0.0; vp[1] = 0.0;
        vp[2] = 0.0; vp[3] = 0.0;
        
        return false;
    }
    
    // crop
    if ( minX < 0. ) minX = 0.;
    if ( minY < 0. ) minY = 0.;
    if ( maxX > 1. ) maxX = 1.;
    if ( maxY > 1. ) maxY = 1.;

    // then finally compute the region 
    vp[0] = minX;
    vp[1] = minY;
    vp[2] = maxX - minX;
    vp[3] = maxY - minY;

    // pixel viewport
    if( pvp != NULL )
    {
        pvp[0] = (int)(vp[0] * _pvp.w + _pvp.x);
        pvp[1] = (int)(vp[1] * _pvp.h + _pvp.y);
        pvp[2] = (int)(vp[2] * _pvp.w);
        pvp[3] = (int)(vp[3] * _pvp.h);
    }
    
    return true;
}
#endif
//---------------------------------------------------------------------------
// initView
//---------------------------------------------------------------------------
template< typename T > 
void Frustum< T >::initView( void )
{
    GLdouble proj[16], model[16];
    GLint pvp[4];

    glGetDoublev( GL_PROJECTION_MATRIX, proj );
    glGetDoublev( GL_MODELVIEW_MATRIX, model );
    glGetIntegerv( GL_VIEWPORT, pvp );

    initView( proj, model, pvp );
}

template< typename T > 
void Frustum< T >::initView( T proj[16], T model[16], 
                             const eq::PixelViewport& pvp )
{
    T   clip[16];
    T   t;

    memcpy( _proj,  proj,  16*sizeof( T ));
    memcpy( _model, model, 16*sizeof( T ));
    _pvp = pvp;

    //===== init frustum

    /* Combine the two matrices (multiply projection by modelview) */
    clip[ 0] = _model[ 0] * _proj[ 0] + _model[ 1] * _proj[ 4] + 
        _model[ 2] * _proj[ 8] + _model[ 3] * _proj[12];
    clip[ 1] = _model[ 0] * _proj[ 1] + _model[ 1] * _proj[ 5] +
        _model[ 2] * _proj[ 9] + _model[ 3] * _proj[13];
    clip[ 2] = _model[ 0] * _proj[ 2] + _model[ 1] * _proj[ 6] +
        _model[ 2] * _proj[10] + _model[ 3] * _proj[14];
    clip[ 3] = _model[ 0] * _proj[ 3] + _model[ 1] * _proj[ 7] +
        _model[ 2] * _proj[11] + _model[ 3] * _proj[15];

    clip[ 4] = _model[ 4] * _proj[ 0] + _model[ 5] * _proj[ 4] +
        _model[ 6] * _proj[ 8] + _model[ 7] * _proj[12];
    clip[ 5] = _model[ 4] * _proj[ 1] + _model[ 5] * _proj[ 5] +
        _model[ 6] * _proj[ 9] + _model[ 7] * _proj[13];
    clip[ 6] = _model[ 4] * _proj[ 2] + _model[ 5] * _proj[ 6] +
        _model[ 6] * _proj[10] + _model[ 7] * _proj[14];
    clip[ 7] = _model[ 4] * _proj[ 3] + _model[ 5] * _proj[ 7] +
        _model[ 6] * _proj[11] + _model[ 7] * _proj[15];

    clip[ 8] = _model[ 8] * _proj[ 0] + _model[ 9] * _proj[ 4] +
        _model[10] * _proj[ 8] + _model[11] * _proj[12];
    clip[ 9] = _model[ 8] * _proj[ 1] + _model[ 9] * _proj[ 5] +
        _model[10] * _proj[ 9] + _model[11] * _proj[13];
    clip[10] = _model[ 8] * _proj[ 2] + _model[ 9] * _proj[ 6] +
        _model[10] * _proj[10] + _model[11] * _proj[14];
    clip[11] = _model[ 8] * _proj[ 3] + _model[ 9] * _proj[ 7] + 
        _model[10] * _proj[11] + _model[11] * _proj[15];

    clip[12] = _model[12] * _proj[ 0] + _model[13] * _proj[ 4] +
        _model[14] * _proj[ 8] + _model[15] * _proj[12];
    clip[13] = _model[12] * _proj[ 1] + _model[13] * _proj[ 5] +
        _model[14] * _proj[ 9] + _model[15] * _proj[13];
    clip[14] = _model[12] * _proj[ 2] + _model[13] * _proj[ 6] +
        _model[14] * _proj[10] + _model[15] * _proj[14];
    clip[15] = _model[12] * _proj[ 3] + _model[13] * _proj[ 7] +
        _model[14] * _proj[11] + _model[15] * _proj[15];
    
    /* Extract the numbers for the RIGHT plane */
    _frustum[0][0] = clip[ 3] - clip[ 0];
    _frustum[0][1] = clip[ 7] - clip[ 4];
    _frustum[0][2] = clip[11] - clip[ 8];
    _frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = sqrt( _frustum[0][0]*_frustum[0][0] + _frustum[0][1]*_frustum[0][1] +
        _frustum[0][2]*_frustum[0][2] );
    _frustum[0][0] /= t;
    _frustum[0][1] /= t;
    _frustum[0][2] /= t;
    _frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    _frustum[1][0] = clip[ 3] + clip[ 0];
    _frustum[1][1] = clip[ 7] + clip[ 4];
    _frustum[1][2] = clip[11] + clip[ 8];
    _frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = sqrt( _frustum[1][0]*_frustum[1][0] + _frustum[1][1]*_frustum[1][1] +
        _frustum[1][2]*_frustum[1][2] );
    _frustum[1][0] /= t;
    _frustum[1][1] /= t;
    _frustum[1][2] /= t;
    _frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    _frustum[2][0] = clip[ 3] + clip[ 1];
    _frustum[2][1] = clip[ 7] + clip[ 5];
    _frustum[2][2] = clip[11] + clip[ 9];
    _frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = sqrt( _frustum[2][0]*_frustum[2][0] + _frustum[2][1]*_frustum[2][1] +
        _frustum[2][2]*_frustum[2][2] );
    _frustum[2][0] /= t;
    _frustum[2][1] /= t;
    _frustum[2][2] /= t;
    _frustum[2][3] /= t;

    /* Extract the TOP plane */
    _frustum[3][0] = clip[ 3] - clip[ 1];
    _frustum[3][1] = clip[ 7] - clip[ 5];
    _frustum[3][2] = clip[11] - clip[ 9];
    _frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = sqrt( _frustum[3][0]*_frustum[3][0] + _frustum[3][1]*_frustum[3][1] +
        _frustum[3][2]*_frustum[3][2] );
    _frustum[3][0] /= t;
    _frustum[3][1] /= t;
    _frustum[3][2] /= t;
    _frustum[3][3] /= t;

    /* Extract the FAR plane */
    _frustum[4][0] = clip[ 3] - clip[ 2];
    _frustum[4][1] = clip[ 7] - clip[ 6];
    _frustum[4][2] = clip[11] - clip[10];
    _frustum[4][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = sqrt( _frustum[4][0]*_frustum[4][0] + _frustum[4][1]*_frustum[4][1] +
        _frustum[4][2]*_frustum[4][2] );
    _frustum[4][0] /= t;
    _frustum[4][1] /= t;
    _frustum[4][2] /= t;
    _frustum[4][3] /= t;

    /* Extract the NEAR plane */
    _frustum[5][0] = clip[ 3] + clip[ 2];
    _frustum[5][1] = clip[ 7] + clip[ 6];
    _frustum[5][2] = clip[11] + clip[10];
    _frustum[5][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = sqrt( _frustum[5][0]*_frustum[5][0] + _frustum[5][1]*_frustum[5][1] +
        _frustum[5][2]*_frustum[5][2] );
    _frustum[5][0] /= t;
    _frustum[5][1] /= t;
    _frustum[5][2] /= t;
    _frustum[5][3] /= t;

    // set dirty flags
    _dirty.screenVec = true;
}

//---------------------------------------------------------------------------
// initScreenVec
//---------------------------------------------------------------------------
#if 0
template< typename T > 
void Frustum< T >::initScreenVec( void )
{
    if( !_dirty.screenVec )
        return;

    //===== init up and right vectors in world coordinates

    GLint    vp[4] = { 0, 0, 1, 1 };
    GLdouble zero[3];

    // defaults
    _screenUp[0] = 1.; 
    _screenUp[1] = 0.; 
    _screenUp[2] = 0.; 
    _screenRight[0] = 0.;
    _screenRight[1] = 1.;
    _screenRight[2] = 0.;

    if( !gluUnProject( 0., 0., 0.,
            _model, _proj, vp,
            &zero[0], &zero[1], &zero[2] ))
        return;
    
    if( gluUnProject( 0., 1., 0.,
            _model, _proj, vp,
            &_screenUp[0], &_screenUp[1], &_screenUp[2] ))
    {
        _screenUp[0] -= zero[0]; 
        _screenUp[1] -= zero[1]; 
        _screenUp[2] -= zero[2]; 
    }

    if( gluUnProject( 1., 0., 0.,
            _model, _proj, vp,
            &_screenRight[0], &_screenRight[1], &_screenRight[2] ))
    {
        _screenRight[0] -= zero[0]; 
        _screenRight[1] -= zero[1]; 
        _screenRight[2] -= zero[2]; 
    }

    normalize( _screenUp );
    normalize( _screenRight );

    _dirty.screenVec = false;
}

//---------------------------------------------------------------------------
// normalize
//---------------------------------------------------------------------------
template< typename T > 
void Frustum< T >::normalize( double vector[3] )
{
    const float scale = sqrt( vector[0]*vector[0] + vector[1]*vector[1] +
        vector[2]*vector[2] );

    assert( scale );

    vector[0] /= scale;
    vector[1] /= scale;
    vector[2] /= scale;
}
#endif
#if 0
// starting point for bounding boxes
bool CubeInFrustum( const float x, const float y,const float z,const float size)
{
   int p;

   for( p = 0; p < 6; p++ )
   {
       if( frustum[p][0] * (x - size) + 
           frustum[p][1] * (y - size) +
           frustum[p][2] * (z - size) +
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x + size) +
           frustum[p][1] * (y - size) + 
           frustum[p][2] * (z - size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x - size) + 
           frustum[p][1] * (y + size) + 
           frustum[p][2] * (z - size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x + size) + 
           frustum[p][1] * (y + size) + 
           frustum[p][2] * (z - size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x - size) + 
           frustum[p][1] * (y - size) + 
           frustum[p][2] * (z + size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x + size) + 
           frustum[p][1] * (y - size) + 
           frustum[p][2] * (z + size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x - size) + 
           frustum[p][1] * (y + size) + 
           frustum[p][2] * (z + size) + 
           frustum[p][3] > 0 )
           continue;

       if( frustum[p][0] * (x + size) + 
           frustum[p][1] * (y + size) + 
           frustum[p][2] * (z + size) + 
           frustum[p][3] > 0 )
           continue;

       return false;
   }

   return true;
}
#endif

#endif // _FRUSTUM_H_
