
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * PURPOSE, AND NONINFRINGEMENT.  PATENT LICENSES, IF ANY, PROVIDED HEREIN    *
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


#include "plyModel.h"

#include <alloca.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#define CHUNKSIZE 4096
#define MAXDEPTH  256

#define PLYFILEVERSION   17

using namespace std;

//---------------------------------------------------------------------------
// PlyModel
//---------------------------------------------------------------------------
template<class FaceType>
PlyModel<FaceType>::PlyModel( void )
        : _nFaces(0),
          _faces(NULL)
{
    _bbox.parent   = NULL;
    _bbox.next     = NULL;
    _bbox.children = NULL;
    _bbox.faces    = NULL;
    _bbox.nFaces   = 0;
}

//---------------------------------------------------------------------------
// ~PlyModel
//---------------------------------------------------------------------------
template<class FaceType>
PlyModel<FaceType>::~PlyModel()
{
    if( _faces != NULL )
        free( _faces );

    freeBBoxes( _bbox );
}

//---------------------------------------------------------------------------
// setFaces
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::setFaces( size_t nFaces, FaceType *faces, 
    size_t bboxFaceThreshold )
{
    if( _faces != NULL )
        free( _faces );

    if( nFaces == 0 )
        return;

    // coordinates
    Vertex bbox[2];
    calculateBBox( nFaces, faces, bbox );

    memcpy( _bbox.pos, bbox, 2*sizeof(Vertex) );
    memcpy( _bbox.cullBox, bbox, 2*sizeof(Vertex) );

    
    _faces    = (FaceType *)malloc( nFaces * sizeof(FaceType) );
    _nFaces = 0;

#ifndef NDEBUG
    fprintf( stderr, "Filling bounding boxes" );
#endif

    _bbox.children = NULL;
    _bbox.parent   = NULL;
    _bbox.next     = NULL;
    _bbox.range[0] = 0.;
    _bbox.range[1] = 1.;

    fillBBox( nFaces, faces, _bbox, bboxFaceThreshold, 0 );

#ifndef NDEBUG
    fprintf( stderr, "\n" );
#endif
    if( _nFaces != nFaces )
        fprintf( stderr, "Error %s:%d: used %zd faces of %zd input faces.\n", 
                __FILE__, __LINE__, nFaces, _nFaces );
}

//---------------------------------------------------------------------------
// calculateBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::calculateBBox( size_t nFaces, FaceType *faces,
    Vertex bbox[2] )
{
    bbox[0].pos[0] = faces[0].vertices[0].pos[0];
    bbox[1].pos[0] = faces[0].vertices[0].pos[0];
    bbox[0].pos[1] = faces[0].vertices[0].pos[1];
    bbox[1].pos[1] = faces[0].vertices[0].pos[1];
    bbox[0].pos[2] = faces[0].vertices[0].pos[2];
    bbox[1].pos[2] = faces[0].vertices[0].pos[2];

    for( size_t i=1; i<nFaces; i++ )
    {
        FaceType &face = faces[i];

        Vertex *vertex = &face.vertices[0];
        if( vertex->pos[0] < bbox[0].pos[0] ) bbox[0].pos[0] = vertex->pos[0];
        if( vertex->pos[1] < bbox[0].pos[1] ) bbox[0].pos[1] = vertex->pos[1];
        if( vertex->pos[2] < bbox[0].pos[2] ) bbox[0].pos[2] = vertex->pos[2];
        
        if( vertex->pos[0] > bbox[1].pos[0] ) bbox[1].pos[0] = vertex->pos[0];
        if( vertex->pos[1] > bbox[1].pos[1] ) bbox[1].pos[1] = vertex->pos[1];
        if( vertex->pos[2] > bbox[1].pos[2] ) bbox[1].pos[2] = vertex->pos[2];

        vertex = &face.vertices[1];
        if( vertex->pos[0] < bbox[0].pos[0] ) bbox[0].pos[0] = vertex->pos[0];
        if( vertex->pos[1] < bbox[0].pos[1] ) bbox[0].pos[1] = vertex->pos[1];
        if( vertex->pos[2] < bbox[0].pos[2] ) bbox[0].pos[2] = vertex->pos[2];
        
        if( vertex->pos[0] > bbox[1].pos[0] ) bbox[1].pos[0] = vertex->pos[0];
        if( vertex->pos[1] > bbox[1].pos[1] ) bbox[1].pos[1] = vertex->pos[1];
        if( vertex->pos[2] > bbox[1].pos[2] ) bbox[1].pos[2] = vertex->pos[2];

        vertex = &face.vertices[2];
        if( vertex->pos[0] < bbox[0].pos[0] ) bbox[0].pos[0] = vertex->pos[0];
        if( vertex->pos[1] < bbox[0].pos[1] ) bbox[0].pos[1] = vertex->pos[1];
        if( vertex->pos[2] < bbox[0].pos[2] ) bbox[0].pos[2] = vertex->pos[2];
        
        if( vertex->pos[0] > bbox[1].pos[0] ) bbox[1].pos[0] = vertex->pos[0];
        if( vertex->pos[1] > bbox[1].pos[1] ) bbox[1].pos[1] = vertex->pos[1];
        if( vertex->pos[2] > bbox[1].pos[2] ) bbox[1].pos[2] = vertex->pos[2];
    }

    // make bbox a bit bigger for faceInBBox test of 'on-the-border' faces
    bbox[1].pos[0] *= 1.01;
    bbox[1].pos[1] *= 1.01;
    bbox[1].pos[2] *= 1.01;
}

//---------------------------------------------------------------------------
// fillBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::fillBBox( size_t nFaces, FaceType *faces, BBox &bbox, 
                                   size_t bboxFaceThreshold, int depth )
{
#ifndef NDEBUG
    static size_t filled = 0;
    if( filled%100 == 0 )
        fprintf( stderr, "." );
    filled++;
#endif

    bbox.nFaces   = 0;
    bbox.faces    = NULL;

    for( size_t i=0; i<nFaces; i++ )
    {
        FaceType &face = faces[i];
        Vertex faceBBox[2];
        calculateFaceBBox( face, faceBBox );
        
        if( faceInBBox( bbox.pos, faceBBox ))
        {
            expandBox( bbox.cullBox, faceBBox );
            addFaceToBBox( bbox, face );
        }
    }

    calculateCullSphere( bbox );

    // update range
    if( nFaces == 0 ) // end pos
        bbox.range[1] = bbox.range[0];
    else
    {
        const float start = bbox.range[0]; // start pos set by previous or init
        const float range = bbox.range[1]; // parents range span set in init

        bbox.range[1] = start + range * bbox.nFaces / nFaces;
    }
    
    if( bbox.next != NULL ) // start range position of next child
        bbox.next->range[0] = bbox.range[1];

    if( bbox.nFaces < bboxFaceThreshold || depth > MAXDEPTH )
    {
        if( bbox.nFaces == 0 )
            return;

        memcpy( &_faces[_nFaces], bbox.faces, bbox.nFaces * sizeof(FaceType) );
        free( bbox.faces );
        bbox.faces = &_faces[_nFaces];
        _nFaces += bbox.nFaces;
    }
    else
    {
        // recursive-fill children bboxes
        createBBoxChildren( bbox );
        
        FaceType *faces = bbox.faces;
        bbox.faces = &_faces[_nFaces];
        
        size_t cnf = 0;

        for( int j=0; j<8; j++ )
        {
            fillBBox( bbox.nFaces, faces, bbox.children[j], bboxFaceThreshold,
                      depth+1 );
            cnf +=  bbox.children[j].nFaces;
        }

        assert( cnf == bbox.nFaces );
        bbox.nFaces = cnf;

        free( faces );
    }
}

//---------------------------------------------------------------------------
// createBBoxChildren
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::createBBoxChildren( BBox &bbox )
{
    bbox.children = (BBox *)calloc( 1, 8*sizeof(BBox) );
    const float range = bbox.range[1] - bbox.range[0];

    for( int i=0; i<8; i++ ) 
    {
        bbox.children[i].parent = &bbox;
        bbox.children[i].range[1] = range;

        if( i<7 )
            bbox.children[i].next = &bbox.children[i+1];
        else
            bbox.children[i].next = NULL;
    }

    // range start (child updates next child when filling bbox)
    bbox.children[0].range[0] = bbox.range[0];

    // child positions
    Vertex *pos = bbox.pos;
    Vertex middle;
    Vertex childPos[2];
    Vertex cullBox[2];

    middle.pos[0] = (pos[0].pos[0] + pos[1].pos[0])/2.;
    middle.pos[1] = (pos[0].pos[1] + pos[1].pos[1])/2.;
    middle.pos[2] = (pos[0].pos[2] + pos[1].pos[2])/2.;

    cullBox[0].pos[0] = FLT_MAX;
    cullBox[0].pos[1] = FLT_MAX;
    cullBox[0].pos[2] = FLT_MAX;
    cullBox[1].pos[0] = -FLT_MAX;
    cullBox[1].pos[1] = -FLT_MAX;
    cullBox[1].pos[2] = -FLT_MAX;

    childPos[0].pos[0] = pos[0].pos[0];
    childPos[0].pos[1] = pos[0].pos[1];
    childPos[0].pos[2] = pos[0].pos[2];
    childPos[1].pos[0] = middle.pos[0];
    childPos[1].pos[1] = middle.pos[1];
    childPos[1].pos[2] = middle.pos[2];
    memcpy( bbox.children[0].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[0].cullBox, cullBox, 2*sizeof(Vertex) );

    childPos[0].pos[0] = middle.pos[0];
    childPos[1].pos[0] = pos[1].pos[0];
    memcpy( bbox.children[1].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[1].cullBox, cullBox, 2*sizeof(Vertex) );

    childPos[0].pos[0] = pos[0].pos[0];
    childPos[1].pos[0] = middle.pos[0];
    childPos[0].pos[1] = middle.pos[1];
    childPos[1].pos[1] = pos[1].pos[1];
    memcpy( bbox.children[2].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[2].cullBox, cullBox, 2*sizeof(Vertex) );
    
    childPos[0].pos[0] = middle.pos[0];
    childPos[1].pos[0] = pos[1].pos[0];
    memcpy( bbox.children[3].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[3].cullBox, cullBox, 2*sizeof(Vertex) );

    childPos[0].pos[0] = pos[0].pos[0];
    childPos[0].pos[1] = pos[0].pos[1];
    childPos[1].pos[0] = middle.pos[0];
    childPos[1].pos[1] = middle.pos[1];
    childPos[0].pos[2] = middle.pos[2];
    childPos[1].pos[2] = pos[1].pos[2];
    memcpy( bbox.children[4].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[4].cullBox, cullBox, 2*sizeof(Vertex) );

    childPos[0].pos[0] = middle.pos[0];
    childPos[1].pos[0] = pos[1].pos[0];
    memcpy( bbox.children[5].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[5].cullBox, cullBox, 2*sizeof(Vertex) );

    childPos[0].pos[0] = pos[0].pos[0];
    childPos[1].pos[0] = middle.pos[0];
    childPos[0].pos[1] = middle.pos[1];
    childPos[1].pos[1] = pos[1].pos[1];
    memcpy( bbox.children[6].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[6].cullBox, cullBox, 2*sizeof(Vertex) );
    
    childPos[0].pos[0] = middle.pos[0];
    childPos[1].pos[0] = pos[1].pos[0];
    memcpy( bbox.children[7].pos, childPos, 2*sizeof(Vertex) );
    memcpy( bbox.children[7].cullBox, cullBox, 2*sizeof(Vertex) );
}

//---------------------------------------------------------------------------
// calculateCullSphere
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::calculateCullSphere( BBox &bbox )
{
    bbox.cullSphere.center.pos[0] = 
        (bbox.cullBox[0].pos[0] + bbox.cullBox[1].pos[0])/2.;
    bbox.cullSphere.center.pos[1] =
        (bbox.cullBox[0].pos[1] + bbox.cullBox[1].pos[1])/2.;
    bbox.cullSphere.center.pos[2] =
        (bbox.cullBox[0].pos[2] + bbox.cullBox[1].pos[2])/2.;

    float v[3];
    v[0] = bbox.cullSphere.center.pos[0] - bbox.cullBox[0].pos[0];
    v[1] = bbox.cullSphere.center.pos[1] - bbox.cullBox[0].pos[1];
    v[2] = bbox.cullSphere.center.pos[2] - bbox.cullBox[0].pos[2];

    bbox.cullSphere.radius = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}   

//---------------------------------------------------------------------------
// calculateFaceBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::calculateFaceBBox( FaceType &face, Vertex bbox[2] )
{
    float *vertex = face.vertices[0].pos;

    bbox[0].pos[0] = vertex[0];
    bbox[1].pos[0] = vertex[0];
    bbox[0].pos[1] = vertex[1];
    bbox[1].pos[1] = vertex[1];
    bbox[0].pos[2] = vertex[2];
    bbox[1].pos[2] = vertex[2];

    vertex = face.vertices[1].pos;
    if( vertex[0] < bbox[0].pos[0] ) bbox[0].pos[0] = vertex[0];
    if( vertex[1] < bbox[0].pos[1] ) bbox[0].pos[1] = vertex[1];
    if( vertex[2] < bbox[0].pos[2] ) bbox[0].pos[2] = vertex[2];
    
    if( vertex[0] > bbox[1].pos[0] ) bbox[1].pos[0] = vertex[0];
    if( vertex[1] > bbox[1].pos[1] ) bbox[1].pos[1] = vertex[1];
    if( vertex[2] > bbox[1].pos[2] ) bbox[1].pos[2] = vertex[2];

    vertex = face.vertices[2].pos;
    if( vertex[0] < bbox[0].pos[0] ) bbox[0].pos[0] = vertex[0];
    if( vertex[1] < bbox[0].pos[1] ) bbox[0].pos[1] = vertex[1];
    if( vertex[2] < bbox[0].pos[2] ) bbox[0].pos[2] = vertex[2];
    
    if( vertex[0] > bbox[1].pos[0] ) bbox[1].pos[0] = vertex[0];
    if( vertex[1] > bbox[1].pos[1] ) bbox[1].pos[1] = vertex[1];
    if( vertex[2] > bbox[1].pos[2] ) bbox[1].pos[2] = vertex[2];
}

//---------------------------------------------------------------------------
// faceInBBox
//  a face is in the bbox if it touches the bbox, and all points are 'bigger'
//  then the bbox min point.
//---------------------------------------------------------------------------
template<class FaceType>
bool PlyModel<FaceType>::faceInBBox( Vertex bbox[2], Vertex faceBBox[2] )
{
    bool touchesBBox = false;

    // first see if it touches the bbox

    if( ( (faceBBox[0].pos[0] < bbox[1].pos[0]) && 
            (faceBBox[1].pos[0] >= bbox[0].pos[0]) ) &&
        ( (faceBBox[0].pos[1] < bbox[1].pos[1]) && 
            (faceBBox[1].pos[1] >= bbox[0].pos[1]) ) &&
        ( (faceBBox[0].pos[2] < bbox[1].pos[2]) && 
            (faceBBox[1].pos[2] >= bbox[0].pos[2]) ) )
        touchesBBox = true;

    if( !touchesBBox )
        return false;

    // see if it is 'left' outside the bbox
    if( faceBBox[0].pos[0] < bbox[0].pos[0] ||
        faceBBox[0].pos[1] < bbox[0].pos[1] ||
        faceBBox[0].pos[2] < bbox[0].pos[2] )
        return false;

    return true;
}

//---------------------------------------------------------------------------
// expandBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::expandBox( Vertex bbox[2], Vertex faceBBox[2])
{
    if( bbox[0].pos[0] > faceBBox[0].pos[0] ) 
        bbox[0].pos[0] = faceBBox[0].pos[0];
    if( bbox[0].pos[1] > faceBBox[0].pos[1] ) 
        bbox[0].pos[1] = faceBBox[0].pos[1];
    if( bbox[0].pos[2] > faceBBox[0].pos[2] ) 
        bbox[0].pos[2] = faceBBox[0].pos[2];

    if( bbox[1].pos[0] < faceBBox[1].pos[0] ) 
        bbox[1].pos[0] = faceBBox[1].pos[0];
    if( bbox[1].pos[1] < faceBBox[1].pos[1] )
        bbox[1].pos[1] = faceBBox[1].pos[1];
    if( bbox[1].pos[2] < faceBBox[1].pos[2] )
        bbox[1].pos[2] = faceBBox[1].pos[2];
}

//---------------------------------------------------------------------------
// addFaceToBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::addFaceToBBox( BBox &bbox, FaceType &face )
{
    if( bbox.nFaces%CHUNKSIZE == 0 )
    {
        if( bbox.faces == NULL )
            bbox.faces = (FaceType *)malloc( CHUNKSIZE*sizeof(FaceType) );
        else
        {
            size_t newSize = (bbox.nFaces/CHUNKSIZE + 1) * 
                CHUNKSIZE*sizeof(FaceType);

            bbox.faces = (FaceType *)realloc( bbox.faces, newSize );
        }
    }

    memcpy( &bbox.faces[bbox.nFaces], &face, sizeof( FaceType ));
    bbox.nFaces++;
}

//---------------------------------------------------------------------------
// scale
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::normalize( void )
{
    Vertex bbox[2];
    memcpy( bbox, _bbox.pos, 2*sizeof(Vertex) );

    scaleModel( bbox );
    traverseBBox( &_bbox, scaleBBoxCB, scaleBBoxCB, NULL, bbox );
}

template<class FaceType>
void PlyModel<FaceType>::scaleModel( Vertex bbox[2] )
{
    const float xScale = bbox[1].pos[0] - bbox[0].pos[0];
    const float yScale = bbox[1].pos[1] - bbox[0].pos[1];
    const float zScale = bbox[1].pos[2] - bbox[0].pos[2];
    float scale = xScale;
    if( yScale > scale ) scale = yScale;
    if( zScale > scale ) scale = zScale;

    const float xOff = (bbox[0].pos[0] + bbox[1].pos[0])/2.;
    const float yOff = (bbox[0].pos[1] + bbox[1].pos[1])/2.;
    const float zOff = (bbox[0].pos[2] + bbox[1].pos[2])/2.;

    for( size_t i=0; i<_nFaces; i++ )
    {        
        _faces[i].vertices[0].pos[0] -= xOff;
        _faces[i].vertices[0].pos[1] -= yOff;
        _faces[i].vertices[0].pos[2] -= zOff;

        _faces[i].vertices[0].pos[0] /= scale;
        _faces[i].vertices[0].pos[1] /= scale;
        _faces[i].vertices[0].pos[2] /= scale;

        _faces[i].vertices[1].pos[0] -= xOff;
        _faces[i].vertices[1].pos[1] -= yOff;
        _faces[i].vertices[1].pos[2] -= zOff;

        _faces[i].vertices[1].pos[0] /= scale;
        _faces[i].vertices[1].pos[1] /= scale;
        _faces[i].vertices[1].pos[2] /= scale;

        _faces[i].vertices[2].pos[0] -= xOff;
        _faces[i].vertices[2].pos[1] -= yOff;
        _faces[i].vertices[2].pos[2] -= zOff;

        _faces[i].vertices[2].pos[0] /= scale;
        _faces[i].vertices[2].pos[1] /= scale;
        _faces[i].vertices[2].pos[2] /= scale;
    }
}

template<class FaceType>
void PlyModel<FaceType>::scaleBBoxCB( PlyModel< FaceType >::BBox *bbox,
    void *data )
{
    Vertex *box = (Vertex *)data;

    const float xScale = box[1].pos[0] - box[0].pos[0];
    const float yScale = box[1].pos[1] - box[0].pos[1];
    const float zScale = box[1].pos[2] - box[0].pos[2];
    float scale = xScale;
    if( yScale > scale ) scale = yScale;
    if( zScale > scale ) scale = zScale;

    const float xOff = (box[0].pos[0] + box[1].pos[0])/2.;
    const float yOff = (box[0].pos[1] + box[1].pos[1])/2.;
    const float zOff = (box[0].pos[2] + box[1].pos[2])/2.;

    
    bbox->cullSphere.center.pos[0] -= xOff;
    bbox->cullSphere.center.pos[1] -= yOff;
    bbox->cullSphere.center.pos[2] -= zOff;

    bbox->cullSphere.center.pos[0] /= scale;
    bbox->cullSphere.center.pos[1] /= scale; 
    bbox->cullSphere.center.pos[2] /= scale;

    bbox->cullSphere.radius /= scale;

    
    bbox->cullBox[0].pos[0] -= xOff;
    bbox->cullBox[0].pos[1] -= yOff;
    bbox->cullBox[0].pos[2] -= zOff;
    bbox->cullBox[1].pos[0] -= xOff;
    bbox->cullBox[1].pos[1] -= yOff;
    bbox->cullBox[1].pos[2] -= zOff;

    bbox->cullBox[0].pos[0] /= scale;
    bbox->cullBox[0].pos[1] /= scale;
    bbox->cullBox[0].pos[2] /= scale;
    bbox->cullBox[1].pos[0] /= scale;
    bbox->cullBox[1].pos[1] /= scale;
    bbox->cullBox[1].pos[2] /= scale;
}

//---------------------------------------------------------------------------
// freeBBoxes
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::freeBBoxes( BBox &bbox )
{
    bbox.faces = NULL;

    if( bbox.children == NULL )
        return;

    for( int i=0; i<8; i++ )
        freeBBoxes( bbox.children[i] );

    free( bbox.children );
    bbox.children = NULL;
}

//---------------------------------------------------------------------------
// traverseBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::traverseBBox( BBox *top, TraverseCB preCB, 
    TraverseCB leafCB, TraverseCB postCB, void *userData )
{
    if ( top->children == NULL ) 
    {
        if ( leafCB ) leafCB( top, userData );
        return;
    }

    BBox *bbox, *next, *parent, *child;
    bbox = top;

    while( true )
    {
        parent = bbox->parent;
        next   = bbox->next;
        child  = (bbox->children==NULL) ? NULL : &bbox->children[0];

        //---------- down-right traversal
        if ( child == NULL ) // leaf
        {
            if ( leafCB ) leafCB( bbox, userData );

            bbox = next;
        } 
        else // node
        {
            if( preCB != NULL ) preCB( bbox, userData );

            bbox = child;
        }

        //---------- up-right traversal
	if( bbox == NULL && parent == NULL ) return;

        while ( bbox == NULL )
        {
            bbox   = parent;
            parent = bbox->parent;
            next   = bbox->next;

            if( postCB ) postCB( bbox, userData );
            
            if ( bbox == top ) return;
            
            bbox = next;
        }
    }
}

//---------------------------------------------------------------------------
// toStream
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel< FaceType >::toStream(ostream& os)
{
    int version = PLYFILEVERSION;
    
    os.write( (char *)&version, sizeof(int) );

    os.write( (char *)&_nFaces, sizeof(size_t) );
    if( _nFaces > 0)
        os.write( (char *)_faces, (ssize_t)(_nFaces*sizeof( FaceType )) );

    writeBBox( os, _bbox );
}

//---------------------------------------------------------------------------
// fromStream
//---------------------------------------------------------------------------

#define mem_read( dst, len ) { memcpy( dst, *addr, len ); *addr += len; }

template<class FaceType>
bool PlyModel<FaceType>::fromMemory( char **addr )
{
    int version;
    mem_read( (char *)&version, sizeof(int) );

    if( version != PLYFILEVERSION )
    {
        fprintf( stderr, "Read version %d, expected %d\n", version,
            PLYFILEVERSION);
        return false;
    }

    mem_read( (char *)&_nFaces, sizeof(size_t) );
    if( _nFaces > 0)
    {
        const size_t size = _nFaces*sizeof( FaceType );
        _faces = (FaceType *)malloc( size );
        
        mem_read( (char *)_faces, size );
    }

    readBBox( addr, _bbox );

    return true;
}

//---------------------------------------------------------------------------
// writeBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::writeBBox( ostream& os, BBox &bbox )
{
    os.write( (char *)&bbox.nFaces, sizeof(size_t) );

    if( bbox.nFaces > 0 )
    {
        size_t ndx = (size_t)( (size_t)((char *)bbox.faces - (char *)_faces) / 
            sizeof(FaceType) );
        
        os.write( (char *)&ndx, sizeof(size_t) );
    }

    os.write( (char *)bbox.pos, 2*sizeof(Vertex) );
    os.write( (char *)bbox.cullBox, 2*sizeof(Vertex) );
    os.write( (char *)&bbox.cullSphere, sizeof(bbox.cullSphere) );
    os.write( (char *)bbox.range, 2*sizeof(float) );

    int nChildren = 8;
    if( bbox.children == NULL )
        nChildren = 0;

    os.write( (char *)&nChildren, sizeof(int) );

    for( int i=0; i<nChildren; i++ )
        writeBBox( os, bbox.children[i] );
}

//---------------------------------------------------------------------------
// readBBox
//---------------------------------------------------------------------------
template<class FaceType>
void PlyModel<FaceType>::readBBox( char** addr, BBox &bbox )
{
    mem_read( (char *)&bbox.nFaces, sizeof(size_t) );

    if( bbox.nFaces > 0 )
    {
        size_t ndx;
        mem_read( (char *)&ndx, sizeof(size_t) );
        bbox.faces = &_faces[ndx];
    }

    mem_read( (char *)bbox.pos, 2*sizeof(Vertex) );
    mem_read( (char *)bbox.cullBox, 2*sizeof(Vertex) );
    mem_read( (char *)&bbox.cullSphere, sizeof(bbox.cullSphere) );
    mem_read( (char *)bbox.range, 2*sizeof(float) );

    int nChildren;
    mem_read( (char *)&nChildren, sizeof(int) );

    if( nChildren == 0 )
        return;

    createBBoxChildren( bbox );

    for( int i=0; i<nChildren; i++ )
        readBBox( addr, bbox.children[i] );
}
