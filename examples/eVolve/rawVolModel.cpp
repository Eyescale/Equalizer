
#include "rawVolModel.h"
#include "hlp.h"

namespace eVolve
{


using hlpFuncs::clip;
using hlpFuncs::hFile;


void createPreintegrationTable( const uint8_t *Table, GLuint &preintName );

int readTrasferFunction( FILE* file,  std::vector<uint8_t>& TF );

int readDimensionsAndScales
( 
    FILE* file, 
    uint32_t& w, uint32_t& h, uint32_t& d, 
    VolumeScales& volScales 
);


// Read volume dimensions, scales and transfer function
RawVolumeModel::RawVolumeModel( const std::string& data ) 
    :_lastSuccess ( false )
    ,_preint      ( 0 )
{
    EQLOG( eq::LOG_CUSTOM ) << "-------------------------------------------"
                            << std::endl;
    EQLOG( eq::LOG_CUSTOM ) << "model: " << _fileName;
    
    _fileName = data;
    hFile header( fopen( ( data + std::string( ".vhf" ) ).c_str(), "rb" ) );

    if( header.f==NULL )
    {
        EQERROR << "Can't open header file" << std::endl;
        return;
    }

    if( readDimensionsAndScales( header.f, _w, _h, _d, _volScales ) )
        return;

    _resolution  = MAX( _w, MAX( _h, _d ) );

    if( readTrasferFunction( header.f, _TF ) != 256 )
        return;

    _lastSuccess = true;
}


int32_t calcHachKey( const eq::Range& range )
{
    return static_cast<int32_t>( range.start*100000000.f + range.end*10000.f );
}

bool RawVolumeModel::getVolumeInfo( VolumeInfo& info, const eq::Range& range )
{
    if( !_lastSuccess )
        return false;

    if( _preint==0 )
    {
        EQLOG( eq::LOG_CUSTOM ) << "Creating preint" << std::endl;
        createPreintegrationTable( &_TF[0], _preint );
    }

    VolumePart* volumePart = NULL;
    int32_t     key        = calcHachKey( range );
    if( _volumeHash.find( key ) == _volumeHash.end( ) )
    {
        //new key
        volumePart = &_volumeHash[ key ];
        if( !_createVolumeTexture( volumePart->volume, volumePart->TD, range ) )
            return false;
    }else // old key
        volumePart = &_volumeHash[ key ];

    info.volume     = volumePart->volume;
    info.TD         = volumePart->TD;
    info.preint     = _preint;
    info.volScales  = _volScales;

    return true;
}


void RawVolumeModel::releaseVolumeInfo( const eq::Range& range )
{
    int32_t key = calcHachKey( range );
    if( _volumeHash.find( key ) == _volumeHash.end() )
        return;

    _volumeHash.erase( key );
}


uint32_t getMinPow2( uint32_t size )
{
    uint32_t res=0;
    
    if( size > 0)
    {
        size--;
        res = 1;
    }
    
    while( size > 0 )
    {
        res  <<= 1;
        size >>= 1;
    }
    return res;
}


bool RawVolumeModel::_createVolumeTexture(        GLuint&    volume,
                                                  DataInTextureDimensions& TD,
                                            const eq::Range& range   )
{
    const uint32_t w = _w;
    const uint32_t h = _h;
    const uint32_t d = _d;

    // reading volume and derivatives
    const int32_t bwStart = 2; //border width from left
    const int32_t bwEnd   = 2; //border width from right

    int32_t s = clip<int32_t>( static_cast<int32_t>( d*range.start ), 0, d-1 );
    int32_t e = clip<int32_t>( static_cast<int32_t>( d*range.end-1 ), 0, d-1 );

    uint32_t start = 
                static_cast<uint32_t>( clip<int32_t>( s-bwStart, 0, d-1 ) );
                
    uint32_t end   = 
                static_cast<uint32_t>( clip<int32_t>( e+bwEnd  , 0, d-1 ) );
                
    uint32_t depth = end-start+1;

    uint32_t tW = getMinPow2( w ); 
    uint32_t tH = getMinPow2( h ); 
    uint32_t tD = getMinPow2( depth );

    EQLOG( eq::LOG_CUSTOM ) << "==============================================="
                            << std::endl
                            << " w: " << w << " " << tW
                            << " h: " << h << " " << tH
                            << " d: " << d << " " << depth << " " << tD
                            << std::endl;

    std::vector<uint8_t> data( tW*tH*tD*4, 0 );

    EQLOG( eq::LOG_CUSTOM ) << "r: " << _resolution << std::endl;
    
    //texture scaling coefficients
    TD.W  = static_cast<float>( w     ) / static_cast<float>( tW );
    TD.H  = static_cast<float>( h     ) / static_cast<float>( tH );
    TD.D  = static_cast<float>( e-s+1 ) / static_cast<float>( tD ); 
    TD.D /= range.end>range.start ? (range.end-range.start) : 1.0 ;

    // Shift coefficient and left border in texture for depth
    TD.Do = range.start;
    TD.Db = range.start > 0.0001 ? bwStart / static_cast<float>(tD) : 0;

    EQLOG( eq::LOG_CUSTOM )
           << " ws: " << TD.W  << " hs: " << TD.H  << " wd: " << TD.D 
           << " Do: " << TD.Do << " Db: " << TD.Db << std::endl 
           << " s= "  << start << " e= "  << end   << std::endl;

    {
        std::ifstream file ( _fileName.c_str(), std::ifstream::in |
                             std::ifstream::binary | std::ifstream::ate );

        if( !file.is_open() ) return _lFailed("Can't open model data file");

        file.seekg( w*h*start*4, std::ios::beg );
        
        if( w==tW && h==tH ) // width and height are power of 2
        {
            file.read( (char*)( &data[0] ), w*h*depth*4 );
        }else
            if( w==tW )     // only width is power of 2
            {
                for( uint32_t i=0; i<depth; i++ )
                    file.read( (char*)( &data[i*tW*tH*4] ), w*h*4 );
            }else
            {               // nor width nor heigh is power of 2
                for( uint32_t i=0; i<depth; i++ )
                    for( uint32_t j=0; j<h; j++ )
                        file.read( (char*)( &data[(i*tW*tH + j*tW)*4] ), w*4 );
            }

        file.close();
    }

    // creating 3D texture
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
    return _lSuccess();
}

/** Volume always represented as cube [-1,-1,-1]..[1,1,1], so if the model 
    is not cube it's proportions should be modified.
*/
void correctScales( uint32_t w, uint32_t h, uint32_t d, VolumeScales &scales )
{
//Correct proportions according to real size of volume
    float maxS = MAX( w, MAX( h, d ) );

    scales.W *= w / maxS;
    scales.H *= h / maxS;
    scales.D *= d / maxS;
    
//Make maximum proportion equal to 1.0 to prevent unnecessary rescaling
    maxS = MAX( scales.W, MAX( scales.H, scales.D ) );

    scales.W /= maxS;
    scales.H /= maxS;
    scales.D /= maxS;
}


int readDimensionsAndScales
( 
    FILE* file, 
    uint32_t& w, uint32_t& h, uint32_t& d, 
    VolumeScales& volScales 
)
{
        fscanf( file, "w=%u\n", &w );
        fscanf( file, "h=%u\n", &h );
    if( fscanf( file, "d=%u\n", &d ) != 1 )
    {
        EQERROR << "Can't read dimensions form header file: "
                << file << std::endl;
        return 1;
    }

        fscanf( file, "wScale=%g\n", &volScales.W );
        fscanf( file, "hScale=%g\n", &volScales.H );
    if( fscanf( file, "dScale=%g\n", &volScales.D ) != 1 )
    {
        EQERROR << "Can't read scales form header file: " 
                << file << std::endl;
        return 1;
    }

    if( w<1 || h<1 || d<1 ||
        volScales.W<0.001 || 
        volScales.H<0.001 || 
        volScales.W<0.001    )
    {
        EQERROR << "volume scales of dimensions incorrect, check: " 
                << file << std::endl;
        return 1;
    }
    
    correctScales( w, h, d, volScales );

    EQLOG( eq::LOG_CUSTOM ) << " " << w <<"x"<< h <<"x"<< d << std::endl;
    EQLOG( eq::LOG_CUSTOM ) << "Scales: "   << volScales.W << " x " 
                                            << volScales.H << " x "
                                            << volScales.D << std::endl;
    return 0;
}


int readTrasferFunction( FILE* file,  std::vector<uint8_t>& TF )
{
    if( fscanf(file,"TF:\n") !=0 )
    {
        EQERROR << "Error in header file";
        return 0;
    }

    uint32_t TFSize;
    fscanf( file, "size=%u\n", &TFSize );

    if( TFSize!=256  )
        EQWARN << "Wrong size of transfer function, should be 256" << std::endl;
        
    TFSize = clip<int32_t>( TFSize, 1, 256 );
    TF.resize( TFSize*4 );

    int tmp;
    for( uint32_t i=0; i<TFSize; i++ )
    {
            fscanf( file, "r=%d\n", &tmp ); TF[4*i  ] = tmp;
            fscanf( file, "g=%d\n", &tmp ); TF[4*i+1] = tmp;
            fscanf( file, "b=%d\n", &tmp ); TF[4*i+2] = tmp;
        if( fscanf( file, "a=%d\n", &tmp ) != 1 )
        {
            EQERROR << "Failed to read entity #" << i 
                    << " of TF from first header file" << std::endl;
            return i;
        }
        TF[4*i+3] = tmp;
    }
    
    return 256;
}


void createPreintegrationTable( const uint8_t *Table, GLuint &preintName )
{
    EQINFO << "Calculating preintegration table..." << std::endl;

    double rInt[256]; rInt[0] = 0.;
    double gInt[256]; gInt[0] = 0.;
    double bInt[256]; bInt[0] = 0.;
    double aInt[256]; aInt[0] = 0.;

    for( int i=1; i<256; i++ )
    {
        double tauc = ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2.;
        
        rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc/255.;
        gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc/255.;
        bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc/255.;
        aInt[i] = aInt[i-1] + tauc;
    }

    std::vector<GLubyte> lookupI( 256*256*4, 255 );

    GLubyte* lookupImg = &lookupI[0]; // Preint Texture
    int lookupindex=0;

    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }
            
            int rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                double factor = 1. / (double)(smax-smin);
                rcol = static_cast<int>( (rInt[smax]-rInt[smin])*factor );
                gcol = static_cast<int>( (gInt[smax]-gInt[smin])*factor );
                bcol = static_cast<int>( (bInt[smax]-bInt[smin])*factor );
#ifdef COMPOSE_MODE_NEW
                acol = static_cast<int>( 
                        256.*(    exp(-(aInt[smax]-aInt[smin])*factor/255.)));
#else
                acol = static_cast<int>(
                        256.*(1.0-exp(-(aInt[smax]-aInt[smin])*factor/255.)));
#endif
            } else
            {
                double factor = 1./255.;
                rcol =
                    static_cast<int>( Table[smin*4+0]*Table[smin*4+3]*factor );

                gcol =
                    static_cast<int>( Table[smin*4+1]*Table[smin*4+3]*factor );
                        
                bcol =
                    static_cast<int>( Table[smin*4+2]*Table[smin*4+3]*factor );
                        
#ifdef COMPOSE_MODE_NEW
                acol =
                    static_cast<int>( 256.*(    exp( -Table[smin*4+3]/255. )));
#else
                acol = 
                    static_cast<int>( 256.*(1.0-exp( -Table[smin*4+3]/255. )));
#endif
            }
            
            lookupImg[lookupindex++] = clip( rcol, 0, 255 );//min( rcol, 255 );
            lookupImg[lookupindex++] = clip( gcol, 0, 255 );//min( gcol, 255 );
            lookupImg[lookupindex++] = clip( bcol, 0, 255 );//min( bcol, 255 );
            lookupImg[lookupindex++] = clip( acol, 0, 255 );//min( acol, 255 );
        }
    }


    glGenTextures( 1, &preintName );
    glBindTexture( GL_TEXTURE_2D, preintName );
    glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, 
                   GL_RGBA, GL_UNSIGNED_BYTE, lookupImg );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
}


}
