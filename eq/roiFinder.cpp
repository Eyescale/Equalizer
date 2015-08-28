
/* Copyright (c)      2009, Maxim Makhinya
 *               2010-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#define EQ_ROI_USE_TRACKER        // disable ROI in case it can't help
//#define EQ_ROI_USE_DEPTH_TEXTURE  // use depth texture instead of color

#include "roiFinder.h"

#ifdef EQ_ROI_USE_DEPTH_TEXTURE
#include "roiFragmentShader_glsl.h"
#else
#include "roiFragmentShaderRGB_glsl.h"
#endif

#include "gl.h"
#include "log.h"

#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>
#include <eq/util/shader.h>
#include <lunchbox/os.h>
#include <pression/plugins/compressor.h>


namespace eq
{

#define glewGetContext glObjects.glewGetContext

// use to address one shader and program per shared context set
static const char seeds = 42;
static const char* shaderRBInfo = &seeds;

#define GRID_SIZE 16 // will be replaced later by variable


ROIFinder::ROIFinder()
    : _dim()
    , _w( 0 )
    , _h( 0 )
    , _wh( 0 )
    , _wb( 0 )
    , _hb( 0 )
    , _wbhb( 0 )
    , _histX()
    , _histY()
{
    _tmpAreas[0].pvp       = PixelViewport( 0, 0, 0, 0 );
    _tmpAreas[0].hole      = PixelViewport( 0, 0, 0, 0 );
    _tmpAreas[0].emptySize = 0;
}

PixelViewport ROIFinder::_getObjectPVP( const PixelViewport& pvp,
                                        const uint8_t*       src )
{
    LBASSERT( pvp.x >= 0 && pvp.x+pvp.w <= _wb &&
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

    // Find AABB based on X and Y axis historgams
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

        // w * h * sizeof( GL_FLOAT ) * RGBA
        _perBlockInfo.resize( _wh * 4 );
    }
}


void ROIFinder::_init( )
{
    _areasToCheck.clear();
    memset( &_mask[0]   , 0, _mask.size( ));

    LBASSERT( static_cast<int32_t>(_perBlockInfo.size()) >= _w*_h*4 );
    LBASSERT( static_cast<int32_t>(_mask.size())         >= _wb*_h  );

    const float*    src = &_perBlockInfo[0];
          uint8_t*  dst = &_mask[0];

    for( int32_t y = 0; y < _h; y++ )
    {
        for( int32_t x = 0; x < _w; x++ )
        {
            if( src[x*4] < 1.0 )
                dst[x] = 255;
        }
        src += _w*4;
        dst += _wb;
    }
}

void ROIFinder::_invalidateAreas( Area* areas, uint8_t num )
{
    for( uint8_t i = 0; i < num; i++ )
        areas[i].valid = false;
}

void ROIFinder::_updateSubArea( const uint8_t type )
{
    LBASSERT( type <= 16 );

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
            LBUNIMPLEMENTED;
    }

    LBASSERT( pvp.hasArea( ));
    LBASSERT( pvp.x >=0 && pvp.y >=0 && pvp.x+pvp.w <=_w && pvp.y+pvp.h <=_h );

    Area& a = _tmpAreas[type];

    a.pvp = _getObjectPVP( pvp, &_mask[0] );

    a.hole = _emptyFinder.getLargestEmptyArea( a.pvp );

    a.emptySize = pvp.getArea() - a.pvp.getArea() + a.hole.getArea();

#ifndef NDEBUG
    LBASSERT( !a.valid );
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
    LBASSERT( a.hole.getArea() > 0 );
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
        LBASSERT( a.pvp.w != a.hole.w );
        type = 8;
    }
    else if( a.pvp.w == a.hole.w ) // hole through the whole block
    {
        type = 9;
    }
    else if( a.pvp.x == a.hole.x ) // left side
    {
        if( a.pvp.y == a.hole.y )
            // in the lower left corner
            type = 0;
        else if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h )
            // in the upper left corner
            type = 1;
        else
            // in the left middle
            type = 2;
    }
    else if( a.pvp.y == a.hole.y ) // bottom side
    {
        if( a.pvp.x + a.pvp.w == a.hole.x + a.hole.w )
            // in the bottom right corner
            type = 3;
        else
            // in the bottom middle
            type = 4;
    }
    else if( a.pvp.x + a.pvp.w == a.hole.x + a.hole.w ) // right side
    {
        if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h )
            // in the upper right corner
            type = 5;
        else
            // in the right middle
            type = 6;
    }
    else if( a.pvp.y + a.pvp.h == a.hole.y + a.hole.h ) // top side
        // in the upper middle corner
        type = 7;
    else
        // must be in the center
        type = 10;

    // Calculate areas of interest
    if( type == 10 ) // center hole position
    {
        for( uint8_t i = 1; i <= 16; i++ )
            _updateSubArea( i );
    }
    else
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
                LBASSERT( _tmpAreas[_compilations16[i][j]].valid );
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
            LBASSERT( _tmpAreas[_compilations16[variant][j]].valid );
            _finalAreas[j] = _tmpAreas[_compilations16[variant][j]];
        }

        return areasPerVariant;
    }
    // else any other hole

    for( uint8_t i = 0; i < varaintsNum; i++ )
    {
        int32_t sum = 0;
        for( uint8_t j = 0; j < areasPerVariant; j++ )
        {
            LBASSERT( _tmpAreas[_compilations[type][i][j]].valid );
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
        LBASSERT( _tmpAreas[_compilations[type][variant][j]].valid );
        _finalAreas[j] = _tmpAreas[_compilations[type][variant][j]];
    }

    return areasPerVariant;
}


void ROIFinder::_findAreas( PixelViewports& resultPVPs )
{
    LBASSERT( _areasToCheck.empty() );

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
    while( !_areasToCheck.empty() )
    {
        Area curArea = _areasToCheck.back();
        _areasToCheck.pop_back();

        uint8_t n = _splitArea( curArea );
        LBASSERT( n >= 2 && n <= 4 );

        for( uint8_t i = 0; i < n; i++ )
        {
            LBASSERT( _finalAreas[i].valid );
            LBASSERT( _finalAreas[i].pvp.hasArea( ));

            if( _finalAreas[i].hole.getArea() == 0 )
                resultPVPs.push_back( _finalAreas[i].pvp );
            else
                _areasToCheck.push_back( _finalAreas[i] );
        }
    }

    // correct position and sizes
    for( uint32_t i = 0; i < resultPVPs.size(); i++ )
    {
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

void ROIFinder::_readbackInfo( util::ObjectManager& glObjects )
{
    LBASSERT( glObjects.supportsEqTexture( ));
    LBASSERT( glObjects.supportsEqFrameBufferObject( ));

    PixelViewport pvp = _pvp;
    pvp.apply( Zoom( GRID_SIZE, GRID_SIZE ));
    pvp.w = LB_MIN( pvp.w+pvp.x, _pvpOriginal.w+_pvpOriginal.x ) - pvp.x;
    pvp.h = LB_MIN( pvp.h+pvp.y, _pvpOriginal.h+_pvpOriginal.y ) - pvp.y;

    LBASSERT( pvp.isValid());

    // copy frame buffer to texture
    const void* bufferKey = _getInfoKey( );
    util::Texture* texture =
        glObjects.obtainEqTexture( bufferKey, GL_TEXTURE_RECTANGLE_ARB );

#ifdef EQ_ROI_USE_DEPTH_TEXTURE
    texture->copyFromFrameBuffer( GL_DEPTH_COMPONENT, pvp );
#else
    texture->copyFromFrameBuffer( GL_RGBA, pvp );
#endif

    // draw zoomed quad into FBO
    const void*     fboKey = _getInfoKey( );
    util::FrameBufferObject* fbo = glObjects.getEqFrameBufferObject( fboKey );

    if( fbo )
    {
        LBCHECK( fbo->resize( _pvp.w, _pvp.h ));
    }
    else
    {
        fbo = glObjects.newEqFrameBufferObject( fboKey );
        LBCHECK( fbo->init( _pvp.w, _pvp.h, GL_RGBA32F, 0, 0 ));
    }
    fbo->bind();

    texture->bind();

    // Enable & download depth texture
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    texture->applyWrap();
    texture->applyZoomFilter( FILTER_LINEAR );

    // Enable shaders
    GLuint program = glObjects.getProgram( shaderRBInfo );
    if( program == util::ObjectManager::INVALID )
    {
        // Create fragment shader which reads depth values from
        // rectangular textures
        const GLuint shader = glObjects.newShader( shaderRBInfo,
                                                        GL_FRAGMENT_SHADER );
        LBASSERT( shader != util::ObjectManager::INVALID );

#ifdef EQ_ROI_USE_DEPTH_TEXTURE
        const GLchar* fShaderPtr = roiFragmentShader_glsl.c_str();
#else
        const GLchar* fShaderPtr = roiFragmentShaderRGB_glsl.c_str();
#endif
        LBCHECK( util::shader::compile( glewGetContext(), shader, fShaderPtr ));
        program = glObjects.newProgram( shaderRBInfo );

        EQ_GL_CALL( glAttachShader( program, shader ));
        EQ_GL_CALL( glLinkProgram( program ));

        GLint status;
        glGetProgramiv( program, GL_LINK_STATUS, &status );
        if( !status )
        {
            LBWARN << "Failed to link shader program for ROI finder"
                   << std::endl;
            return;
        }

        // use fragment shader and setup uniforms
        EQ_GL_CALL( glUseProgram( program ));

        GLint param = glGetUniformLocation( program, "texture" );
        glUniform1i( param, 0 );
    }
    else
    {
        // use fragment shader
        EQ_GL_CALL( glUseProgram( program ));
    }

    // Draw Quad
    glDisable( GL_LIGHTING );
    glColor3f( 1.0f, 1.0f, 1.0f );

    glBegin( GL_QUADS );
        glVertex3i(      0,      0, 0 );
        glVertex3i( _pvp.w,      0, 0 );
        glVertex3i( _pvp.w, _pvp.h, 0 );
        glVertex3i(      0, _pvp.h, 0 );
    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));

    fbo->unbind();

    // finish readback of info
    LBASSERT( static_cast<int32_t>(_perBlockInfo.size()) >= _pvp.w*_pvp.h*4 );

    texture = fbo->getColorTextures()[0];
    LBASSERT( texture->getFormat() == GL_RGBA );
    LBASSERT( texture->getType() == GL_FLOAT );
    texture->download( &_perBlockInfo[0] );
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


PixelViewports ROIFinder::findRegions( const uint32_t         buffers,
                                       const PixelViewport&   pvp,
                                       const Zoom&            zoom,
                                       const uint32_t         stage,
                                       const uint128_t&       frameID,
                                       util::ObjectManager&   glObjects )
{
    PixelViewports result;
    result.push_back( pvp );

    LBLOG( LOG_ASSEMBLY ) << "ROIFinder::getObjects " << pvp << ", buffers "
                          << buffers << std::endl;

    if( zoom != Zoom::NONE )
    {
        LBWARN << "R-B optimization impossible when zoom is used"
               << std::endl;
        return result;
    }

#ifdef EQ_ROI_USE_TRACKER
    uint8_t* ticket;
    if( !_roiTracker.useROIFinder( pvp, stage, frameID, ticket ))
        return result;
#endif

    _pvpOriginal = pvp;
    _resize( _getBoundingPVP( pvp ));

    // go through depth buffer and check min/max/BG values
    // render to and read-back usefull info from FBO
    _readbackInfo( glObjects );
    glObjects.clear();

    // Analyze readed back data and find regions of interest
    _init( );

    _emptyFinder.update( &_mask[0], _wb, _hb );
    _emptyFinder.setLimits( 200, 0.002f );

    result.clear();
    _findAreas( result );

#ifdef EQ_ROI_USE_TRACKER
    _roiTracker.updateDelay( result, ticket );
#endif

    return result;
}

}
