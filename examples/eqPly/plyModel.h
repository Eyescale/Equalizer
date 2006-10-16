
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

#ifndef __PLYMODEL_H
#define __PLYMODEL_H

// template class for managing polygonal data
// uses bounding box octtree for data representation

#include "vertex.h"

#include <eq/vmmlib/Vector3.h>
#include <fstream>

template<class FaceType> class PlyModel
{
public:

    PlyModel( void );
    ~PlyModel();

    // model data
    struct BBox 
    {
        BBox   *parent;
        BBox   *next;
        BBox   *children;  // eight or NULL

        Vertex  pos[2];
        Vertex  cullBox[2];

        struct {
            Vertex center;
            float  radius;
        } cullSphere;

        size_t     nFaces;
        FaceType  *faces;

        float      range[2];

        void      *userData;
    };

    typedef void (*TraverseCB)( BBox *bbox, void *userData );

    // methods
    void setFaces( size_t nFaces, FaceType *faces, size_t bboxFaceThreshold );
    void normalize( void );

    void toStream( std::ostream& os );
    bool fromMemory( char* addr );

    const BBox *getBBox( void ) const { return &_bbox; }

    static void traverseBBox( BBox* bbox, TraverseCB preCB, TraverseCB leafCB, 
        TraverseCB postCB, void *userData );

private:

    // model data -- flat
    size_t      _nFaces;
    size_t      _faceSize;
    FaceType   *_faces;

    // model data -- bbox octtree
    BBox        _bbox;

    // methods
    void writeBBox( std::ostream& os, BBox &bbox );
    void readBBox( char** addr, BBox &bbox );

    void calculateBBox( size_t nFaces, FaceType *faces, Vertex bbox[2] );

    void fillBBox( size_t nFaces, FaceType *faces, BBox &bbox, 
                   size_t bboxFaceThreshold, int depth );
    void createBBoxChildren( BBox &bbox );

    void calculateCullSphere( BBox &bbox );
    void calculateFaceBBox( FaceType &face, Vertex bbox[2]);

    bool faceInBBox( Vertex bbox[2], Vertex faceBBox[2] );
    void expandBox( Vertex bbox[2], Vertex faceBBox[2] );
    void addFaceToBBox( BBox &bbox, FaceType &face );

    static float _getScaleFactor( const Vertex bbox[2] );
    void        scaleModel( const float scale, const vmml::Vector3f& offset );
    static void scaleBBoxCB( PlyModel< FaceType >::BBox *bbox, void *data );

    void freeBBoxes( BBox &bbox );
};

#ifdef __GNUC__
   // including the entire templated implementation for template instantation
#  include "plyModel.cpp"
#endif

#endif // __PLYMODEL_H

