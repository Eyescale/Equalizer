
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rawVolModel.h"
#include "hlp.h"

namespace eVolve
{


using hlpFuncs::clip;
using hlpFuncs::hFile;


static GLuint createPreintegrationTable( const uint8_t* Table );

static bool readTransferFunction( FILE* file, std::vector<uint8_t>& TF );

static bool readDimensionsAndScaling
( 
    FILE* file, 
    uint32_t& w, uint32_t& h, uint32_t& d, 
    VolumeScaling& volScaling 
);


// Read volume dimensions, scaling and transfer function
RawVolumeModel::RawVolumeModel( const std::string& filename  )
        : _headerLoaded( false )
        , _filename( filename )
        , _preintName  ( 0 )
        , _glewContext( 0 )
{}

bool RawVolumeModel::loadHeader( const float brightness, const float alpha )
{
    EQASSERT( !_headerLoaded );
    EQASSERT( brightness > 0.0f );
    EQASSERT( alpha > 0.0f );
    EQLOG( eq::LOG_CUSTOM ) << "-------------------------------------------"
                            << std::endl << "model: " << _filename;

    hFile header( fopen( ( _filename + std::string( ".vhf" ) ).c_str(), "rb" ));

    if( header.f==0 )
    {
        EQERROR << "Can't open header file" << std::endl;
        return false;
    }

    if( !readDimensionsAndScaling( header.f, _w, _h, _d, _volScaling ) )
        return false;

    _resolution = EQ_MAX( _w, EQ_MAX( _h, _d ) );

    if( !readTransferFunction( header.f, _TF ))
        return false;

    _headerLoaded = true;

    if( brightness != 1.0f )
    {
        for( size_t i = 0; i < _TF.size(); i+=4 )
        {
            _TF[i+0] = static_cast< uint8_t >( _TF[i+0] * brightness );
            _TF[i+1] = static_cast< uint8_t >( _TF[i+1] * brightness );
            _TF[i+2] = static_cast< uint8_t >( _TF[i+2] * brightness );
        }
    }

    if( alpha != 1.0f )
        for( size_t i = 3; i < _TF.size(); i+=4 )
            _TF[i] = static_cast< uint8_t >( _TF[i] * alpha );

    return true;
}


static int32_t calcHashKey( const eq::Range& range )
{
    return static_cast<int32_t>(( range.start*10000.f + range.end )*10000.f );
}


bool RawVolumeModel::getVolumeInfo( VolumeInfo& info, const eq::Range& range )
{
    if( !_headerLoaded && !loadHeader( 1.0f, 1.0f ))
        return false;

    if( _preintName == 0 )
    {
        EQLOG( eq::LOG_CUSTOM ) << "Creating preint" << std::endl;
        _preintName = createPreintegrationTable( &_TF[0] );
    }

          VolumePart* volumePart = 0;
    const int32_t     key        = calcHashKey( range );

    if( _volumeHash.find( key ) == _volumeHash.end( ) )
    {
        // new key
        volumePart = &_volumeHash[ key ];
        if( !_createVolumeTexture( volumePart->volume, volumePart->TD, range ))
            return false;
    }
    else
    {   // old key
        volumePart = &_volumeHash[ key ];
    }

    info.volume     = volumePart->volume;
    info.TD         = volumePart->TD;
    info.preint     = _preintName;
    info.volScaling  = _volScaling;

    return true;
}


void RawVolumeModel::releaseVolumeInfo( const eq::Range& range )
{
    const int32_t key = calcHashKey( range );
    if( _volumeHash.find( key ) == _volumeHash.end() )
        return;

    _volumeHash.erase( key );
}


/** Calculates minimal power of 2 which is greater than given number
*/
static uint32_t calcMinPow2( uint32_t size )
{
    if( size == 0 )
        return 0;
    
    size--;
    uint32_t res = 1;

    while( size > 0 )
    {
        res  <<= 1;
        size >>= 1;
    }
    return res;
}


/** Reading requested part of volume and derivatives from data file
*/
bool RawVolumeModel::_createVolumeTexture(        GLuint&    volume,
                                                  DataInTextureDimensions& TD,
                                            const eq::Range& range    )
{
    const uint32_t w = _w;
    const uint32_t h = _h;
    const uint32_t d = _d;

    const int32_t bwStart = 2; //border width from left
    const int32_t bwEnd   = 2; //border width from right

    const int32_t s =
            clip<int32_t>( static_cast< int32_t >( d*range.start ), 0, d-1 );

    const int32_t e =
            clip<int32_t>( static_cast< int32_t >( d*range.end-1 ), 0, d-1 );

    const uint32_t start =
                static_cast<uint32_t>( clip<int32_t>( s-bwStart, 0, d-1 ) );

    const uint32_t end   =
                static_cast<uint32_t>( clip<int32_t>( e+bwEnd  , 0, d-1 ) );

    const uint32_t depth = end-start+1;

    const uint32_t tW = calcMinPow2( w );
    const uint32_t tH = calcMinPow2( h );
    const uint32_t tD = calcMinPow2( depth );

    //texture scaling coefficients
    TD.W  = static_cast<float>( w     ) / static_cast<float>( tW );
    TD.H  = static_cast<float>( h     ) / static_cast<float>( tH );
    TD.D  = static_cast<float>( e-s+1 ) / static_cast<float>( tD );
    TD.D /= range.end>range.start ? (range.end-range.start) : 1.0f;

    // Shift coefficient and left border in texture for depth
    TD.Do = range.start;
    TD.Db = range.start > 0.0001 ? bwStart / static_cast<float>(tD) : 0;

    EQLOG( eq::LOG_CUSTOM )
            << "==============================================="   << std::endl
            << " w: "  << w << " " << tW
            << " h: "  << h << " " << tH
            << " d: "  << d << " " << depth << " " << tD           << std::endl
            << " r: "  << _resolution                              << std::endl
            << " ws: " << TD.W  << " hs: " << TD.H  << " wd: " << TD.D 
            << " Do: " << TD.Do << " Db: " << TD.Db                << std::endl
            << " s= "  << start << " e= "  << end                  << std::endl;

    // Reading of requested part of a volume
    std::vector<uint8_t> data( tW*tH*tD*4, 0 );
    const uint32_t  wh4 =  w *  h * 4;
    const uint32_t tWH4 = tW * tH * 4;

    std::ifstream file ( _filename.c_str(), std::ifstream::in |
                         std::ifstream::binary | std::ifstream::ate );

    if( !file.is_open() )
    {
        EQERROR << "Can't open model data file";
        return false;
    }

    file.seekg( wh4*start, std::ios::beg );

    if( w==tW && h==tH ) // width and height are power of 2
    {
        file.read( (char*)( &data[0] ), wh4*depth );
    }
    else if( w==tW )     // only width is power of 2
    {
        for( uint32_t i=0; i<depth; i++ )
            file.read( (char*)( &data[i*tWH4] ), wh4 );
    }
    else
    {               // nor width nor heigh is power of 2
        const uint32_t   w4 =  w * 4;
        const uint32_t  tW4 = tW * 4;
        
        for( uint32_t i=0; i<depth; i++ )
            for( uint32_t j=0; j<h; j++ )
                file.read( (char*)( &data[ i*tWH4 + j*tW4] ), w4 );
    }

    file.close();

    EQASSERT( _glewContext );
    // create 3D texture
    glGenTextures( 1, &volume );
    EQLOG( eq::LOG_CUSTOM ) << "generated texture: " << volume << std::endl;
    glBindTexture(GL_TEXTURE_3D, volume);

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );

    glTexImage3D(   GL_TEXTURE_3D, 
                    0, GL_RGBA, tW, tH, tD, 
                    0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(&data[0]) );
    return true;
}


/** Volume always represented as cube [-1,-1,-1]..[1,1,1], so if the model 
    is not cube it's proportions should be modified. This function makes 
    maximum proportion equal to 1.0 to prevent unnecessary rescaling.
*/
static void normalizeScaling
(
    const uint32_t       w,
    const uint32_t       h,
    const uint32_t       d,
          VolumeScaling& scaling
)
{
//Correct proportions according to real size of volume
    float maxS = float( EQ_MAX( w, EQ_MAX( h, d ) ));

    scaling.W *= w / maxS;
    scaling.H *= h / maxS;
    scaling.D *= d / maxS;

//Make maximum proportion equal to 1.0
    maxS = EQ_MAX( scaling.W, EQ_MAX( scaling.H, scaling.D ) );

    scaling.W /= maxS;
    scaling.H /= maxS;
    scaling.D /= maxS;
}


static bool readDimensionsAndScaling
( 
    FILE* file, 
    uint32_t& w, uint32_t& h, uint32_t& d, 
    VolumeScaling& volScaling 
)
{
    if( fscanf( file, "w=%u\n", &w ) == EOF )
        return false;
    if( fscanf( file, "h=%u\n", &h ) == EOF )
        return false;
    if( fscanf( file, "d=%u\n", &d ) != 1 )
    {
        EQERROR << "Can't read dimensions from header file" << std::endl;
        return false;
    }

    if( fscanf( file, "wScale=%g\n", &volScaling.W ) == EOF )
        return false;
    if( fscanf( file, "hScale=%g\n", &volScaling.H ) == EOF )
        return false;
    if( fscanf( file, "dScale=%g\n", &volScaling.D ) != 1 )
    {
        EQERROR << "Can't read scaling from header file: " << std::endl;
        return false;
    }

    if( w<1 || h<1 || d<1 ||
        volScaling.W<0.001 || 
        volScaling.H<0.001 || 
        volScaling.W<0.001    )
    {
        EQERROR << "volume scaling is incorrect, check header file"<< std::endl;
        return false;
    }

    normalizeScaling( w, h, d, volScaling );

    EQLOG( eq::LOG_CUSTOM ) << " "  << w << "x" << h << "x" << d << std::endl
                            << "( " << volScaling.W << " x "
                                    << volScaling.H << " x "
                                    << volScaling.D << " )"       << std::endl;
    return true;
}


static bool readTransferFunction( FILE* file,  std::vector<uint8_t>& TF )
{
    if( fscanf(file,"TF:\n") !=0 )
    {
        EQERROR << "Error in header file, can't find 'TF:' marker.";
        return false;
    }

    uint32_t TFSize;
    if( fscanf( file, "size=%u\n", &TFSize ) == EOF )
        return false;

    if( TFSize!=256  )
        EQWARN << "Wrong size of transfer function, should be 256" << std::endl;

    TFSize = clip<int32_t>( TFSize, 1, 256 );
    TF.resize( TFSize*4 );

    int tmp;
    for( uint32_t i=0; i<TFSize; i++ )
    {
        if( fscanf( file, "r=%d\n", &tmp ) == EOF )
            return false;
        TF[4*i  ] = tmp;
        if( fscanf( file, "g=%d\n", &tmp ) == EOF )
            return false;
        TF[4*i+1] = tmp;
        if( fscanf( file, "b=%d\n", &tmp ) == EOF )
            return false;
        TF[4*i+2] = tmp;
        if( fscanf( file, "a=%d\n", &tmp ) != 1 )
        {
            EQERROR << "Failed to read entity #" << i 
                    << " of TF from header file" << std::endl;
            return i;
        }
        TF[4*i+3] = tmp;
    }
    return true;
}


static GLuint createPreintegrationTable( const uint8_t *Table )
{
    EQLOG( eq::LOG_CUSTOM ) << "Calculating preintegration table" << std::endl;

    double rInt[256]; rInt[0] = 0.;
    double gInt[256]; gInt[0] = 0.;
    double bInt[256]; bInt[0] = 0.;
    double aInt[256]; aInt[0] = 0.;

    // Creating SAT (Summed Area Tables) from averaged neighbouring RGBA values
    for( int i=1; i<256; i++ )
    {
        // average Alpha from two neighbouring TF values
        const double tauc =   ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2. / 255.;

        // SAT of average RGBs from two neighbouring TF values 
        // multiplied with Alpha
        rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc;
        gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc;
        bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc;

        // SAT of average Alpha values
        aInt[i] = aInt[i-1] + tauc;
    }

    std::vector<GLubyte> lookupImg( 256*256*4, 255 ); // Preint Texture

    int lookupindex=0;

    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }

            double rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                const double factor = 1. / (double)(smax-smin);
                rcol =       ( rInt[smax] - rInt[smin] ) * factor;
                gcol =       ( gInt[smax] - gInt[smin] ) * factor;
                bcol =       ( bInt[smax] - bInt[smin] ) * factor;
                acol = exp( -( aInt[smax] - aInt[smin] ) * factor) * 255.;
            } else
            {
                const int    index  = smin*4;
                const double factor = 1./255.;
                rcol =       Table[index+0] * Table[index+3] * factor;
                gcol =       Table[index+1] * Table[index+3] * factor;
                bcol =       Table[index+2] * Table[index+3] * factor;
                acol = exp(                  -Table[index+3] * factor ) * 256.;
            }
            lookupImg[lookupindex++] = clip( static_cast<int>(rcol), 0, 255 );
            lookupImg[lookupindex++] = clip( static_cast<int>(gcol), 0, 255 );
            lookupImg[lookupindex++] = clip( static_cast<int>(bcol), 0, 255 );
            lookupImg[lookupindex++] = clip( static_cast<int>(acol), 0, 255 );
        }
    }

    GLuint preintName = 0;
    glGenTextures( 1, &preintName );
    glBindTexture( GL_TEXTURE_2D, preintName );
    glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, 
                   GL_RGBA, GL_UNSIGNED_BYTE, &lookupImg[0] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

    return preintName;
}


}
