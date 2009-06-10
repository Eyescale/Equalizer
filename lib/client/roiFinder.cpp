
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

//#define EQ_USE_ROI
//#define EQ_USE_DEPTH_TEXTURE

#include "roiFinder.h"

#ifdef EQ_USE_DEPTH_TEXTURE
#include "roiFragmentShader_glsl.h"
#else 
#include "roiFragmentShaderRGB_glsl.h"
#endif

#include "frameBufferObject.h"
#include "log.h"

#include <eq/base/base.h>


namespace eq
{

// use to address one shader and program per shared context set
static const char seeds = 42;
static const char* shaderRBInfo = &seeds;

#define GRID_SIZE 16 // will be replaced later by variable


ROIFinder::ROIFinder()
    : _glObjects( 0 )
{
    _tmpAreas[0].pvp       = PixelViewport( 0, 0, 0, 0 );
    _tmpAreas[0].hole      = PixelViewport( 0, 0, 0, 0 );
    _tmpAreas[0].emptySize = 0;
}

void ROIFinder::_dumpDebug( const std::string file )
{
    _tmpImg.reset();
    _tmpImg.setPixelViewport( PixelViewport( 0, 0, _wb, _hb ));

    _tmpImg.setFormat( Frame::BUFFER_COLOR, GL_RGB  );
    _tmpImg.setType(   Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );

    _tmpImg.validatePixelData( Frame::BUFFER_COLOR );

    uint8_t* dst  = _tmpImg.getPixelPointer( Frame::BUFFER_COLOR );
    uint8_t* src1 = &_mask[0];
    uint8_t* src2 = &_tmpMask[0];

    memset( dst, 0, _wbhb*3 );

    for( int32_t y = 0; y < _hb; y++ )
        for( int32_t x = 0; x < _wb; x++ )
        {
            dst[0] = *src1++;
            dst[1] = *src2++;
            dst += 3;
        }

    _tmpImg.writeImages( file );
}


void ROIFinder::_calcHist( const PixelViewport& pvp, const uint8_t* src )
{
    EQASSERT( pvp.x >= 0 && pvp.x+pvp.w <= _wb &&
              pvp.y >= 0 && pvp.y+pvp.h <= _hb );

    // Calculate per-pixel histograms
    const uint8_t* s = src + pvp.y*_wb + pvp.x;

    memset( _histX, 0, pvp.w );
    memset( _histY, 0, pvp.h );
    for( int32_t y = 0; y < pvp.h; y++ )
    {
        for( int32_t x = 0; x < pvp.w; x++ )
        {
            const uint8_t val = s[ x ] & 1;
            _histX[ x ] += val;
            _histY[ y ] += val;
        }
        s += _wb;
    }

    // equalize histogram values
    for( int32_t x = 0; x < pvp.w; x++ )
        _histX[ x ] = _histX[ x ] * 255 / pvp.h;

    for( int32_t y = 0; y < pvp.h; y++ )
        _histY[ y ] = _histY[ y ] * 255 / pvp.w;
}


PixelViewport ROIFinder::_getObjectPVP( const PixelViewport& pvp,
                                              const uint8_t* src )
{
    _calcHist( pvp, src );

    int32_t xMin = pvp.w;
    for( int32_t x = 0; x < pvp.w; x++ )
        if( _histX[x] != 0 )
        {
            xMin = x;
            break;
        }

    int32_t xMax = 0;
    for( int32_t x = pvp.w-1; x >= 0; x-- )
        if( _histX[x] != 0 )
        {
            xMax = x;
            break;
        }

    if( xMax < xMin )
        return PixelViewport( pvp.x, pvp.y, 0, 0 );

    int32_t yMin = pvp.h;
    for( int32_t y = 0; y < pvp.h; y++ )
        if( _histY[y] != 0 )
        {
            yMin = y;
            break;
        }

    int32_t yMax = 0;
    for( int32_t y = pvp.h-1; y >= 0; y-- )
        if( _histY[y] != 0 )
        {
            yMax = y;
            break;
        }

    if( yMax < yMin )
        return PixelViewport( pvp.x, pvp.y, 0, 0 );

    return PixelViewport( pvp.x+xMin, pvp.y+yMin, xMax-xMin+1, yMax-yMin+1 );
}


void ROIFinder::_resize( const PixelViewport& pvp )
{
    _pvp = pvp;

    _w   = _pvp.w;
    _h   = _pvp.h;
    _wh  = _w * _h;
    _wb  = _w + 1; // borders are only on left and 
    _hb  = _h + 1; // top borders of the image
    _wbhb = _wb * _hb;

    if( static_cast<int32_t>(_mask.size()) < _wbhb )
    {
        _mask.resize( _wbhb );
        _tmpMask.resize( _wbhb );

        // w * h * sizeof( GL_FLOAT ) * RGBA
        _perBlockInfo.resize( _wh * 4 );
    }
}


void ROIFinder::_init( )
{
    _areasToCheck.clear();
    memset( &_mask[0]   , 0, _mask.size( ));
    memset( &_tmpMask[0], 0, _tmpMask.size( ));

    const float*    src = &_perBlockInfo[0];
          uint8_t*  dst = &_mask[0];

    for( int32_t y = 0; y < _h; y++ )
    {
        for( int32_t x = 0; x < _w; x++ )
        {
            if( src[x*4] < 1.0 )
                dst[x] = 255;
        }
        dst += _wb;
        src += _w*4;
    }
}


void ROIFinder::_fillWithColor( const PixelViewport& pvp,
                                      uint8_t* dst, const uint8_t val )
{
    for( int32_t y = pvp.y; y < pvp.y + pvp.h; y++ )
        for( int32_t x = pvp.x; x < pvp.x + pvp.w; x++ )
            dst[ y * _wb + x ] = val; 
}

void ROIFinder::_invalidateAreas( Area* areas, uint8_t num )
{
    for( uint8_t i = 0; i < num; i++ )
        areas[i].valid = false;
}

void ROIFinder::_updateSubArea( const uint8_t type )
{
    EQASSERT( type <= 16 );

    if( type == 0 )
        return;

    PixelViewport pvp;
    switch( type )
    {
        case 1:  pvp = PixelViewport( _dim.x1,_dim.y2,_dim.w1,_dim.h2 ); break;
        case 2:  pvp = PixelViewport( _dim.x2,_dim.y3,_dim.w2,_dim.h3 ); break;
        case 3:  pvp = PixelViewport( _dim.x3,_dim.y2,_dim.w3,_dim.h2 ); break;
        case 4:  pvp = PixelViewport( _dim.x2,_dim.y1,_dim.w2,_dim.h1 ); break;
        case 5:  pvp = PixelViewport( _dim.x1,_dim.y1,_dim.w1,_dim.h4 ); break;
        case 6:  pvp = PixelViewport( _dim.x1,_dim.y3,_dim.w4,_dim.h3 ); break;
        case 7:  pvp = PixelViewport( _dim.x3,_dim.y2,_dim.w3,_dim.h5 ); break;
        case 8:  pvp = PixelViewport( _dim.x2,_dim.y1,_dim.w5,_dim.h1 ); break;
        case 9:  pvp = PixelViewport( _dim.x1,_dim.y2,_dim.w1,_dim.h5 ); break;
        case 10: pvp = PixelViewport( _dim.x2,_dim.y3,_dim.w5,_dim.h3 ); break;
        case 11: pvp = PixelViewport( _dim.x3,_dim.y1,_dim.w3,_dim.h4 ); break;
        case 12: pvp = PixelViewport( _dim.x1,_dim.y1,_dim.w4,_dim.h1 ); break;
        case 13: pvp = PixelViewport( _dim.x1,_dim.y1,_dim.w1,_dim.h6 ); break;
        case 14: pvp = PixelViewport( _dim.x3,_dim.y1,_dim.w3,_dim.h6 ); break;
        case 15: pvp = PixelViewport( _dim.x1,_dim.y3,_dim.w6,_dim.h3 ); break;
        case 16: pvp = PixelViewport( _dim.x1,_dim.y1,_dim.w6,_dim.h1 ); break;
        default:
            EQUNIMPLEMENTED;
    }

    EQASSERT( pvp.hasArea( ));
    EQASSERT( pvp.x >=0 && pvp.y >=0 && pvp.x+pvp.w <=_w && pvp.y+pvp.h <=_h );

    Area& a = _tmpAreas[type];

    a.pvp = _getObjectPVP( pvp, &_mask[0] );

    a.hole = _emptyFinder.getLargestEmptyArea( a.pvp );

    a.emptySize = pvp.getArea() - a.pvp.getArea() + a.hole.getArea();

    EQASSERT( !a.valid );

#ifndef NDEBUG
    a.valid = true;
#endif
}


// positions of a hole:
//
//  1 7 5
//  2 8 6
//  0 4 3
//
// 0, 1, 3, 5 - corners
// 2, 4, 6, 7 - sides
// 8          - center
//


static const uint8_t _interests[10][8] =
{
    { 2, 3, 7,10, 0, 0, 0, 0 }, // corner
    { 3, 4, 8,11, 0, 0, 0, 0 }, // corner
    { 2, 3, 4, 7, 8,10,11,14 }, // side
    { 1, 2, 6, 9, 0, 0, 0, 0 }, // corner
    { 1, 2, 3, 6, 7, 9,10,15 }, // side
    { 1, 4, 5,12, 0, 0, 0, 0 }, // corner
    { 1, 2, 4, 5, 6, 9,12,13 }, // side
    { 1, 3, 4, 5, 8,11,12,16 },  // side
    { 1, 3, 0, 0, 0, 0, 0, 0 }, // vertical
    { 2, 4, 0, 0, 0, 0, 0, 0 }, // horizontal
};                              // center is 1..16

static const uint8_t _compilNums[11][2] =
{
    {2,2},{2,2},{4,3},{2,2},{4,3},{2,2},{4,3},{4,3},{1,2},{1,2},{18,4}
};

static const uint8_t _compilations[10][4][3] =
{
    {{2, 7, 0},{3,10, 0},{0,0, 0},{0,0, 0}}, // corner
    {{3, 8, 0},{4,11, 0},{0,0, 0},{0,0, 0}}, // corner
    {{2, 4,14},{4,10,11},{3,8,10},{2,7, 8}}, // side
    {{1, 6, 0},{2, 9, 0},{0,0, 0},{0,0, 0}}, // corner
    {{1, 3,15},{3, 9,10},{2,7, 9},{1,6, 7}}, // side
    {{1,12, 0},{4, 5, 0},{0,0, 0},{0,0, 0}}, // corner
    {{2, 4,13},{4, 5, 6},{1,6,12},{2,9,12}}, // side
    {{1, 3,16},{1,11,12},{5,4,11},{3,5, 8}}, // side
    {{1, 3, 0},{0, 0, 0},{0,0, 0},{0,0, 0}}, // vertical
    {{2, 4, 0},{0, 0, 0},{0,0, 0},{0,0, 0}}, // horizontal
};


static const uint8_t _compilations16[18][4] =// center
{
    {13,2, 4,14},{13,4,10,11},{13,3,8,10},{13,2,7, 8},
    {16,1, 3,15},{16,3, 9,10},{16,2,7, 9},{16,1,6, 7},
    {14,2, 4,13},{14,4, 5, 6},{14,1,6,12},{14,2,9,12},
    {15,1, 3,16},{15,1,11,12},{15,5,4,11},{15,3,5, 8},
    {5 ,6, 7, 8},{9,10,11,12}
};


uint8_t ROIFinder::_splitArea( Area& a )
{
    EQASSERT( a.hole.getArea() > 0 );
#ifndef NDEBUG
    _invalidateAreas( _tmpAreas, 17 );
#endif

    _dim.x1 = a.pvp.x;
    _dim.x2 = a.hole.x;
    _dim.x3 = a.hole.x + a.hole.w;

    _dim.w1 = _dim.x2 - _dim.x1;
    _dim.w2 = a.hole.w;
    _dim.w3 = a.pvp.x + a.pvp.w - _dim.x3;
    _dim.w4 = _dim.w1 + _dim.w2;
    _dim.w5 = _dim.w2 + _dim.w3;
    _dim.w6 = _dim.w4 + _dim.w3;

    _dim.y1 = a.pvp.y;
    _dim.y2 = a.hole.y;
    _dim.y3 = a.hole.y + a.hole.h;

    _dim.h1 = _dim.y2 - _dim.y1;
    _dim.h2 = a.hole.h;
    _dim.h3 = a.pvp.y + a.pvp.h - _dim.y3;
    _dim.h4 = _dim.h1 + _dim.h2;
    _dim.h5 = _dim.h2 + _dim.h3;
    _dim.h6 = _dim.h4 + _dim.h3;

    // other cases
    uint8_t type;
    if( a.pvp.h == a.hole.h ) // hole through the whole block
    {
        EQASSERT( a.pvp.w != a.hole.w );
        type = 8;
    }else
    if( a.pvp.w == a.hole.w ) // hole through the whole block
    {
        type = 9;
    }else
    if( a.pvp.x == a.hole.x ) // left side
    {
        if( a.pvp.y == a.hole.y )
        {// in the lower left corner
            type = 0;
        }else
        if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h )
        {// in the upper left corner
            type = 1;
        }else
        {// in the left middle
            type = 2;
        }
    }else
    if( a.pvp.y == a.hole.y ) // bottom side
    {
        if( a.pvp.x + a.pvp.w == a.hole.x + a.hole.w )
        {// in the bottom right corner
            type = 3;
        }else
        {// in the bottom middle
            type = 4;
        }
    }else
    if( a.pvp.x + a.pvp.w == a.hole.x + a.hole.w ) // right side
    {
        if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h )
        {// in the upper right corner
            type = 5;
        }else
        {// in the right middle
            type = 6;
        }
    }else
    if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h ) // top side
    {// in the upper middle corner
            type = 7;
    }else
    {// must be in the center
            type = 10;
    }

    // Calculate areas of interest
    if( type == 10 ) // center hole position
    {
        for( uint8_t i = 1; i <= 16; i++ )
            _updateSubArea( i );
    }else
    {
        for( uint8_t i = 0; i < 8; i++ )
            _updateSubArea( _interests[ type ][ i ] );
    }

    // find best combinations of areas of interest
    const uint8_t varaintsNum     = _compilNums[type][0];
    const uint8_t areasPerVariant = _compilNums[type][1];

    int32_t maxSum  = 0;
    int32_t variant = 0;
    if( type == 10 ) // center hole
    {
        for( uint8_t i = 0; i < varaintsNum; i++ )
        {
            int32_t sum = 0;
            for( uint8_t j = 0; j < areasPerVariant; j++ )
            {
                EQASSERT( _tmpAreas[_compilations16[i][j]].valid );
                sum += _tmpAreas[_compilations16[i][j]].emptySize;
            }

            if( sum > maxSum )
            {
                maxSum  = sum;
                variant = i;
            }
        }

        for( uint8_t j = 0; j < areasPerVariant; j++ )
        {
            EQASSERT( _tmpAreas[_compilations16[variant][j]].valid );
            _finalAreas[j] = &_tmpAreas[_compilations16[variant][j]];
        }

        return areasPerVariant;
    }
    // else any other hole

    for( uint8_t i = 0; i < varaintsNum; i++ )
    {
        int32_t sum = 0;
        for( uint8_t j = 0; j < areasPerVariant; j++ )
        {
            EQASSERT( _tmpAreas[_compilations[type][i][j]].valid );
            sum += _tmpAreas[_compilations[type][i][j]].emptySize;
        }

        if( sum > maxSum )
        {
            maxSum  = sum;
            variant = i;
        }
    }

    for( uint8_t j = 0; j < areasPerVariant; j++ )
    {
        EQASSERT( _tmpAreas[_compilations[type][variant][j]].valid );
        _finalAreas[j] = &_tmpAreas[_compilations[type][variant][j]];
    }

    return areasPerVariant;
}


void ROIFinder::_findAreas( PixelViewportVector& resultPVPs )
{
    EQASSERT( _areasToCheck.size() == 0 );

    Area area( PixelViewport( 0, 0, _w, _h ));
    area.pvp  = _getObjectPVP( area.pvp, &_mask[0] );

    if( area.pvp.w <= 0 || area.pvp.h <= 0 )
        return;

    area.hole = _emptyFinder.getLargestEmptyArea( area.pvp );

    if( area.hole.getArea() == 0 )
        resultPVPs.push_back( area.pvp );
    else
        _areasToCheck.push_back( area );

    // try to split areas
    while( _areasToCheck.size() > 0 )
    {
        Area curArea = _areasToCheck.back();
        _areasToCheck.pop_back();

        uint8_t n = _splitArea( curArea );
        EQASSERT( n >= 2 && n <= 4 );

        for( uint8_t i = 0; i < n; i++ )
        {
            EQASSERT( _finalAreas[i]->valid );
            EQASSERT( _finalAreas[i]->pvp.hasArea( ));
            
            if( _finalAreas[i]->hole.getArea() == 0 )
                resultPVPs.push_back( _finalAreas[i]->pvp );
            else
                _areasToCheck.push_back( *_finalAreas[i] );
        }
    }

    // correct position and sizes
    for( uint32_t i = 0; i < resultPVPs.size(); i++ )
    {
        _fillWithColor( resultPVPs[i], &_tmpMask[0], 
                        255 - i*200/resultPVPs.size( ));

        PixelViewport& pvp = resultPVPs[i];
        pvp.x += _pvp.x;
        pvp.y += _pvp.y;

        pvp.apply( Zoom( GRID_SIZE, GRID_SIZE ));
    }

}

const void* ROIFinder::_getInfoKey( ) const
{
    return ( reinterpret_cast< const char* >( this ) + 3 );
}


void ROIFinder::_readbackInfo( )
{
    EQASSERT( _glObjects );
    EQASSERT( _glObjects->supportsEqTexture( ));
    EQASSERT( _glObjects->supportsEqFrameBufferObject( ));

    PixelViewport pvp = _pvp;
    pvp.apply( Zoom( GRID_SIZE, GRID_SIZE ));

    // copy frame buffer to texture
    const void* bufferKey = _getInfoKey( );
    Texture*    texture   = _glObjects->obtainEqTexture( bufferKey );

#ifdef EQ_USE_DEPTH_TEXTURE
    texture->setFormat( GL_DEPTH_COMPONENT );
#else
    texture->setFormat( GL_RGBA );
#endif

    texture->copyFromFrameBuffer( pvp );

    // draw zoomed quad into FBO
    const void*     fboKey = _getInfoKey( );
    FrameBufferObject* fbo = _glObjects->getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        EQCHECK( fbo->resize( _pvp.w, _pvp.h ));
    }
    else
    {
        fbo = _glObjects->newEqFrameBufferObject( fboKey );
        fbo->setColorFormat( GL_RGBA32F );
        EQCHECK( fbo->init( _pvp.w, _pvp.h, 0, 0 ));
    }
    fbo->bind();

    // Enable & download depth texture
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
                                                            GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                                                            GL_CLAMP_TO_EDGE );

    texture->bind();

    // Enable shaders
    //
    GLuint program = _glObjects->getProgram( shaderRBInfo );

    if( program == Window::ObjectManager::INVALID )
    {
        // Create fragment shader which reads depth values from 
        // rectangular textures
        const GLuint shader = _glObjects->newShader( shaderRBInfo,
                                                        GL_FRAGMENT_SHADER );
        EQASSERT( shader != Window::ObjectManager::INVALID );

#ifdef EQ_USE_DEPTH_TEXTURE
        const GLchar* fShaderPtr = roiFragmentShader_glsl.c_str();
#else
        const GLchar* fShaderPtr = roiFragmentShaderRGB_glsl.c_str();
#endif
        EQ_GL_CALL( glShaderSource( shader, 1, &fShaderPtr, 0 ));
        EQ_GL_CALL( glCompileShader( shader ));

        GLint status;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
        if( !status )
            EQERROR << "Failed to compile fragment shader for ROI finder"
                    << std::endl;

        program = _glObjects->newProgram( shaderRBInfo );

        EQ_GL_CALL( glAttachShader( program, shader ));
        EQ_GL_CALL( glLinkProgram( program ));

        glGetProgramiv( program, GL_LINK_STATUS, &status );
        if( !status )
        {
            EQWARN << "Failed to link shader program for ROI finder"
                   << std::endl;
            return;
        }

        // use fragment shader and setup uniforms
        EQ_GL_CALL( glUseProgram( program ));

        const GLint param = glGetUniformLocation( program, "texture" );
        glUniform1i( param, 0 );
    }
    else
    {
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));
    }

    // Draw Quad
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glColor3f( 1.0f, 1.0f, 1.0f );

    const float tEndX = (_pvp.w-1)*16.0f+1.0f;
    const float tEndY = (_pvp.h-1)*16.0f+1.0f;;
    const float vEndX = static_cast< float >( _pvp.w );
    const float vEndY = static_cast< float >( _pvp.h );

    glBegin( GL_QUADS );
        glTexCoord2f(  0.0f, 0.0f );
        glVertex3f(    0.0f, 0.0f, 0.0f );

        glTexCoord2f( tEndX, 0.0f );
        glVertex3f(   vEndX, 0.0f, 0.0f );

        glTexCoord2f( tEndX, tEndY );
        glVertex3f(   vEndX, vEndY, 0.0f );

        glTexCoord2f(  0.0f, tEndY );
        glVertex3f(    0.0f, vEndY, 0.0f );
    glEnd();


    // restore state
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));

    fbo->unbind();

    // finish readback of info
    texture = fbo->getColorTextures()[0];
    texture->download( &_perBlockInfo[0], GL_RGBA, GL_FLOAT );
}


static PixelViewport _getBoundingPVP( const PixelViewport& pvp )
{
    PixelViewport pvp_;

    pvp_.x = ( pvp.x / GRID_SIZE );
    pvp_.y = ( pvp.y / GRID_SIZE );

    pvp_.w = (( pvp.x + pvp.w + GRID_SIZE-1 )/GRID_SIZE ) - pvp_.x;
    pvp_.h = (( pvp.y + pvp.h + GRID_SIZE-1 )/GRID_SIZE ) - pvp_.y;

    return pvp_;
}


PixelViewportVector ROIFinder::findRegions( const uint32_t         buffers,
                                            const PixelViewport&   pvp,
                                            const Zoom&            zoom,
                                            const uint32_t         stage,
                                            const uint32_t         frameID,
                                            Window::ObjectManager* glObjects )
{
    PixelViewportVector result;
    result.push_back( pvp );

#ifndef EQ_USE_ROI
    return result; // disable read back info usage
#endif

/*
    eq::base::Clock clock;
    clock.reset();
for( int i = 0; i < 100; i++ )
{
*/
    EQASSERT( glObjects );
    EQASSERTINFO( !_glObjects, "Another readback in progress?" );
    EQLOG( LOG_ASSEMBLY )   << "ROIFinder::getObjects " << pvp
                            << ", buffers " << buffers
                            << std::endl;

    if( zoom != Zoom::NONE )
    {
        EQWARN << "R-B optimization impossible when zoom is used"
               << std::endl;
        return result;
    }

    if( !(buffers & Frame::BUFFER_DEPTH ))
    {
        EQWARN << "No depth buffer, R-B optimization impossible" 
               << std::endl;
        return result;
    }

/*    uint8_t* ticket;
    if( !_roiTracker.useROIFinder( pvp, stage, frameID, ticket ))
        return result;
*/
    _resize( _getBoundingPVP( pvp ));

    // go through depth buffer and check min/max/BG values
    // render to and read-back usefull info from FBO

    _glObjects = glObjects;
    _readbackInfo();
    _glObjects = 0;

    // Analyze readed back data and find regions of interest

    _init( );

    _emptyFinder.update( &_mask[0], _wb, _hb );
    _emptyFinder.setLimits( 200, 0.02f );
    
    result.clear();
    _findAreas( result );
//    _roiTracker.updateDelay( result, ticket );

/*
}


    const float time = clock.getTimef() / 100;
    const float fps  = 1000.f / time;

    static float minFPS = 10000;
    static float maxFPS = 0;
    if( minFPS > fps ) minFPS = fps;
    if( maxFPS < fps ) maxFPS = fps;

    EQWARN << "=============================================" << std::endl;
    EQWARN << "ROI min fps: " << minFPS << " (" << 1000.f/minFPS
          << " ms) max fps: " << maxFPS << " (" << 1000.f/maxFPS
          << " ms) areas found: " << result.size() << std::endl;
*/

/*
    static uint32_t counter = 0;
    std::ostringstream ss;
    ss << "_img_" << ++counter;
    _dumpDebug( ss.str( ) + "_00" );
*/
//    EQWARN << "Areas found: " << result.size() << std::endl;
    return result;
}

}

