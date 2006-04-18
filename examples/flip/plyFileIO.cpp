
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
 * PURPOSE, AND NONINFRINGEMENT.  PATENT LICENSES, IF ANY, PROVIDED HEREIN   *
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

#include "plyFileIO.h"

#include "plyModel.h"
#include "colorVertex.h"
#include "normalFace.h"

#include <eq/eq.h>

#include <alloca.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace eqBase;

//---------------------------------------------------------------------------
// read
//---------------------------------------------------------------------------
PlyModel< NormalFace<ColorVertex> > *PlyFileIO::read( const char *filename )
{
    const size_t len     = strlen(filename);
    char        *binName = (char *)alloca( (unsigned int)len+6 );
    const bool   isFile = isPlyFile( filename );

    if( isFile )
    {
        sprintf( binName, "%s", filename );
        binName[len-3] = 'b';
        if( sizeof( __SIZE_TYPE__ ) == 8 )
        {
            binName[len-2] = '6';
            binName[len-1] = '4';
        }
        else
        {
            binName[len-2] = '3';
            binName[len-1] = '2';
        }
    }
    else
    {
        if( sizeof( __SIZE_TYPE__ ) == 8 )
            sprintf( binName, "%s/.b64", filename ); // assumes directory
        else
            sprintf( binName, "%s/.b32", filename ); // assumes directory
    }

#if 1
    PlyModel< NormalFace<ColorVertex> > *model = readBin( binName );
#else
    PlyModel< NormalFace<ColorVertex> > *model = NULL;
#endif

    if( model == NULL )
    {
        if( isFile )
            model = readPly( filename );
        else
            model = readDir( filename );

        if( model == NULL )
            return NULL;
        
        writeBin( model, binName );
    }

    return model;
}

//---------------------------------------------------------------------------
// readDir
//---------------------------------------------------------------------------
PlyModel< NormalFace<ColorVertex> > *PlyFileIO::readDir( const char *dirname )
{
    vector<NormalFace<ColorVertex>  *> faces;
    vector<size_t>                     nFaces;

    readDirRec( dirname, faces, nFaces );

    if( faces.size() == 0 )
        return NULL;


    PlyModel< NormalFace<ColorVertex> > *model = 
        new PlyModel< NormalFace<ColorVertex> >;

    size_t nAllFaces = 0;

    for( size_t i=0; i<nFaces.size(); i++ )
        nAllFaces += nFaces[i];

    NormalFace<ColorVertex>  *allFaces = (NormalFace<ColorVertex> *)
        malloc( nAllFaces * sizeof( NormalFace<ColorVertex> ));

    NormalFace<ColorVertex> *ptr = allFaces;

    for( size_t i=0; i<faces.size(); i++ )
    {
        const size_t size = nFaces[i] * sizeof( NormalFace<ColorVertex> );
        memcpy( ptr, faces[i], size );
        ptr += nFaces[i];

        free( faces[i] );
    }
    
    size_t threshold = (nAllFaces<16000) ? 16 : nAllFaces/1000;
    threshold = (threshold>4096) ? 4096 : threshold;

    model->setFaces( nAllFaces, allFaces, threshold );
    model->normalize();

    free( allFaces );    
    return model;
}

void PlyFileIO::readDirRec( const char *dirname, 
    vector<NormalFace<ColorVertex> *> &faces, vector<size_t> &nFaces )
{
    DIR *dirp = opendir( dirname );
    if (dirp == NULL) return;

    struct dirent *de;
    const size_t   dirNameLen = strlen( dirname );

    while( (de = readdir(dirp)) != NULL )
    {
        const unsigned len    = (unsigned)( strlen( de->d_name )+dirNameLen+2 );
        char        *filename = (char *)alloca( len );
        
        sprintf( filename, "%s/%s", dirname, de->d_name );

        if( isPlyFile( de->d_name ))
        {
            NormalFace<ColorVertex>  *mFaces;
            size_t                    mNFaces;

            if( readPlyFile( filename, &mFaces, &mNFaces ))
            {
                faces.push_back( mFaces );
                nFaces.push_back( mNFaces );
            }
        }
        else
        {
            if( strcmp( de->d_name, "." ) != 0 &&
                strcmp( de->d_name, ".." ) != 0 )
            {
                readDirRec( filename, faces, nFaces );
            }
        }
    }

    closedir(dirp);
}

//---------------------------------------------------------------------------
// isPlyFile
//---------------------------------------------------------------------------
bool PlyFileIO::isPlyFile( const char *filename )
{
    const size_t len = strlen( filename );
    
    if( len > 4 &&
        filename[len-4] == '.' &&
        filename[len-3] == 'p' &&
        filename[len-2] == 'l' &&
        filename[len-1] == 'y' )
        return true;
    
    return false;
}
//---------------------------------------------------------------------------
// readPly
//---------------------------------------------------------------------------
PlyModel< NormalFace<ColorVertex> > *PlyFileIO::readPly( const char *filename )
{
    NormalFace<ColorVertex>  *faces    = NULL;
    size_t                    nFaces;

    if( !readPlyFile( filename, &faces, &nFaces ))
        return NULL;

    PlyModel< NormalFace<ColorVertex> > *model = 
        new PlyModel< NormalFace<ColorVertex> >;

    size_t threshold = (nFaces<16000) ? 16 : nFaces/1000;
    threshold = (threshold>4096) ? 4096 : threshold;
    model->setFaces( nFaces, faces, threshold );
    model->normalize();

    free( faces );    
    return model;
}

bool PlyFileIO::readPlyFile( const char *filename,
    NormalFace<ColorVertex> **faces, size_t *nFaces )
{
    int       nElems;
    char    **elemNames;
    int       fileType;
    float     version;

    PlyFile *file = ply_open_for_reading( (char *)filename, &nElems, 
        &elemNames, &fileType, &version );

    if( file == NULL )
        return false;

    ColorVertex *vertices  = NULL;
    int          nVertices = 0;

    EQINFO << filename << ": " << nElems << " elements, file version " 
           << version << endl;

    for( int i=0; i<nElems; i++ )
    {
        int nElems, nProps;

        PlyProperty **props = 
            ply_get_element_description( file, elemNames[i], &nElems, &nProps );

        EQINFO << "Element " << i << ": name " << elemNames[i] << ", " 
               << nProps << " properties" << endl;

        // vertices
        if( strcmp( elemNames[i], "vertex" ) == 0 )
        {
            bool color = false;
            // look for color in model
            for( int j=0; j<nProps; j++ )
                if( strcmp( props[j]->name, "red" )==0 )
                    color = true;
            
            vertices = (ColorVertex *)malloc( nElems * sizeof( ColorVertex ));
            readVertices( file, nElems, color, vertices );
            nVertices = nElems;
        }

        // faces
        else if( strcmp( elemNames[i], "face" ) == 0 )
        {
            assert( vertices );
            *faces = (NormalFace<ColorVertex> *)
                malloc( nElems * sizeof( NormalFace<ColorVertex> ));

            readFaces( file, vertices, nVertices, *faces, nElems );

            *nFaces = nElems;
            free( vertices );
            nVertices = 0;
            ply_close( file );

            return true;
        }
    }

    ply_close( file );
    return false;
}

//---------------------------------------------------------------------------
// readVertices
//---------------------------------------------------------------------------
void PlyFileIO::readVertices( PlyFile *file, int num, bool color, 
    ColorVertex *vertices )
{
    // setup read
    int nVProps = 3;
    
    // C++ does not allow offsetof on non-POD types. Work around this using
    // pointer voodoo knowing that our non-POD ColorVertex is 'well-behaved'
    ColorVertex dummy;
    const int xOffset = ((char*)&dummy.pos[0] - (char*)&dummy );
    const int yOffset = xOffset + (int)sizeof(int);
    const int zOffset = yOffset + (int)sizeof(int);

    PlyProperty vProps[6] = { 
        {"x",PLY_FLOAT,PLY_FLOAT, xOffset, 0,0,0,0},
        {"y",PLY_FLOAT,PLY_FLOAT, yOffset, 0,0,0,0},
        {"z",PLY_FLOAT,PLY_FLOAT, zOffset, 0,0,0,0},
    };

    if( color )
    {
        for( int i=3; i<6; i++ )
        {
            vProps[i].name = (char *)alloca( 32 );
            vProps[i].external_type = PLY_UCHAR;   
            vProps[i].internal_type = PLY_FLOAT;   
            
            vProps[i].is_list = 0;         
        }

        sprintf( vProps[3].name, "red" );
        sprintf( vProps[4].name, "green" );
        sprintf( vProps[5].name, "blue" );

        // non-POD offsets: see comment above
        const int rOffset = ((char*)&dummy.color[0] - (char*)&dummy );
        const int gOffset = rOffset + (int)sizeof(float);
        const int bOffset = gOffset + (int)sizeof(float);
        
        vProps[3].offset = rOffset;
        vProps[4].offset = gOffset;
        vProps[5].offset = bOffset;        
        nVProps = 6;
    }
        
    EQINFO << "Reading " << num << " vertices";

    ply_get_element_setup( file, "vertex", nVProps, vProps );
            
    // read vertices
    for( int i=0; i<num; i++ )
    {
        if( i%10000 == 0 )
            EQINFO << "." << flush;

        ply_get_element( file, &vertices[i] );

        if( color )
        {
            vertices[i].color[0] /= 256.;
            vertices[i].color[1] /= 256.;
            vertices[i].color[2] /= 256.;
        }
        else
        {
            vertices[i].color[0] = 1.;
            vertices[i].color[1] = 1.;
            vertices[i].color[2] = 1.;
        }
    }
    EQINFO << endl;
}

//---------------------------------------------------------------------------
// readFaces
//---------------------------------------------------------------------------
void PlyFileIO::readFaces( PlyFile *file, 
                           ColorVertex *vertices, const int nVertices, 
                           NormalFace<ColorVertex> *faces, const int nFaces )
{
    // setup read
    int nFProps = 1;

    struct IndexFace {
        int nVertices;
        int *vertices;
    } face;

    PlyProperty fProps[] = { 
        "vertex_indices", PLY_INT, PLY_INT, offsetof( IndexFace, vertices ),
        1, PLY_INT, PLY_INT, offsetof( IndexFace, nVertices ) };

    EQINFO << "Reading " << nFaces << " faces";

    ply_get_element_setup( file, "face", nFProps, fProps );
            
    // read faces
    int wrongNormals = 0;
    for( int i=0; i<nFaces; i++ )
    {
        if( i%10000 == 0 )
            EQINFO << "." << flush;

        // read face
        ply_get_element( file, &face );
        assert( face.nVertices == 3 );

        // dereference face indices
        for( int j=0; j<3; j++ )
        {
            assert( face.vertices[j] < nVertices );
            faces[i].vertices[j] = vertices[face.vertices[j]]; 
        }
        
        // calculate normal
        if( !calculateNormal( faces[i] ))
            ++wrongNormals;
    }
#ifndef NDEBUG
    EQINFO << endl;

    if( wrongNormals )
        EQWARN << "No normal for " << wrongNormals << " faces ("
               << (float)wrongNormals/(float)nFaces*100. << "%)" << endl;
#endif            
}

//---------------------------------------------------------------------------
// calculateNormal
//---------------------------------------------------------------------------
bool PlyFileIO::calculateNormal( NormalFace<ColorVertex> &face )
{
    double ax = 1000.*face.vertices[0].pos[0] - 1000.*face.vertices[1].pos[0];
    double ay = 1000.*face.vertices[0].pos[1] - 1000.*face.vertices[1].pos[1];
    double az = 1000.*face.vertices[0].pos[2] - 1000.*face.vertices[1].pos[2];
    
    double bx = 1000.*face.vertices[0].pos[0] - 1000.*face.vertices[2].pos[0];
    double by = 1000.*face.vertices[0].pos[1] - 1000.*face.vertices[2].pos[1];
    double bz = 1000.*face.vertices[0].pos[2] - 1000.*face.vertices[2].pos[2];
    
    face.normal[0] = ay*bz - az*by;
    face.normal[1] = az*bx - ax*bz;
    face.normal[2] = ax*by - ay*bx;
                    
    double n = sqrt( face.normal[0] * face.normal[0] + 
                     face.normal[1] * face.normal[1] + 
                     face.normal[2] * face.normal[2] );
    
    if( n==0. )
    {
        face.normal[0] = 0.;
        face.normal[1] = 0.;
        face.normal[2] = 1.;
        return false;
    }

    face.normal[0] /= n;
    face.normal[1] /= n;
    face.normal[2] /= n;
    return true;
}

//---------------------------------------------------------------------------
// readBin
//---------------------------------------------------------------------------
PlyModel< NormalFace<ColorVertex> > *PlyFileIO::readBin( const char *filename )
{
    int fd = open( filename, O_RDONLY );
    if ( fd < 0 ) return false;

    struct stat status;

    fstat( fd, &status );
    char *addr = (char *) mmap( NULL, (size_t) status.st_size, PROT_READ,
            MAP_SHARED, fd, 0 );

    if( addr == MAP_FAILED )
        return NULL;

    PlyModel< NormalFace<ColorVertex> > *model = 
        new PlyModel< NormalFace<ColorVertex> >;

    if( !model->fromMemory( &addr ) )
    {
        delete model;
        model = NULL;
    }

    munmap( addr, (size_t) status.st_size );
    close(fd);

    return model;
}

//---------------------------------------------------------------------------
// writeBin
//---------------------------------------------------------------------------
void PlyFileIO::writeBin( PlyModel< NormalFace<ColorVertex> > *model, 
    const char *filename )
{
    ofstream fout( filename, ios::out );
    if( !fout )
        return;

    model->toStream( fout );
}
