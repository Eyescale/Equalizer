/*
 * Copyright (c) 2007, Maxim Makhinya
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

#include "ddsbase.h"

#include <math.h>
#ifndef _MSC_VER
#  include <stdint.h>
#endif

#define EQ_MIN(a,b) ((a)<(b)?(a):(b))
#ifndef MIN
#  define MIN EQ_MIN
#endif
#include <tclap/CmdLine.h>

#include "eVolveConverter.h"
#include "hlp.h"

#define QUOTE( string ) STRINGIFY( string )
#define STRINGIFY( foo ) #foo

int main( int argc, char** argv )
{
    eVolve::RawConverter::parseArguments( argc, argv );
}

namespace eVolve
{

using namespace std;
using hlpFuncs::clip;
using hlpFuncs::min;
using hlpFuncs::hFile;


#define LBERROR cerr
#define LBWARN  cout


static int lFailed( const char* msg, int result=1 )
{ LBERROR << msg << endl; return result; }


int RawConverter::parseArguments( int argc, char** argv )
{
    try
    {
        TCLAP::CmdLine command( "eVolveConverter - eVolve file converter",
                                ' ', QUOTE( EQUALIZER_VERSION ));
        TCLAP::ValueArg<double> sclXArg( "", "sW", "scale factor for width",
                                         false, 1.0  , "double", command );
        TCLAP::ValueArg<double> sclYArg( "", "sH", "scale factor for height",
                                         false, 1.0  , "double", command );
        TCLAP::ValueArg<double> sclZArg( "", "sD", "scale factor for depth",
                                         false, 1.0  , "double", command );
        TCLAP::ValueArg<double> sclArg( "", "sA", "common scale factor",
                                        false, 1.0  , "double", command );
        TCLAP::SwitchArg recArg( "e", "rec",
                                 "recalculate derivatives in raw+der",
                                 command, false );
        TCLAP::SwitchArg cmpArg( "m", "cmp", "compare two raw+derivations+vhf",
                                 command, false );
        TCLAP::SwitchArg dscArg( "c", "dsc", "dsc->vhf file converter",
                                 command, false );
        TCLAP::SwitchArg savArg( "v", "sav",
                                 "sav->vhf transfer function converter",
                                 command, false );
        TCLAP::SwitchArg derArg( "r", "der", "raw->raw + derivatives",
                                 command, false );
        TCLAP::SwitchArg rawArg( "w", "raw", "raw + derivatives->raw",
                                 command, false );
        TCLAP::SwitchArg pvmArg( "p", "pvm", "pvm[+sav]->raw+derivatives+vhf",
                                 command, false );
        TCLAP::ValueArg<string> dstArg( "d", "dst", "destination file", true,
                                        "Bucky32x32x32_d.raw", "string",
                                        command );
        TCLAP::ValueArg<string> srcArg( "s", "src", "source file", true,
                                        "Bucky32x32x32.raw", "string",
                                        command );
        command.parse( argc, argv );

        if( rawArg.isSet() ) // raw + derivatives -> raw
            return RawConverter::RawPlusDerivativesToRawConverter(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( derArg.isSet() ) // raw -> raw + derivatives
            return RawConverter::RawToRawPlusDerivativesConverter(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( savArg.isSet() ) // sav -> vhf
            return RawConverter::SavToVhfConverter(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( dscArg.isSet() ) // dsc -> vhf
            return RawConverter::DscToVhfConverter(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( pvmArg.isSet() ) // pvm -> raw
            return RawConverter::PvmSavToRawDerVhfConverter(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( cmpArg.isSet() ) // cmp raw+derivations+vhf
            return RawConverter::CompareTwoRawDerVhf(
                        srcArg.getValue( ), dstArg.getValue( ));

        if( recArg.isSet() ) // recalculate derivatives
            return RawConverter::RecalculateDerivatives(
                        srcArg.getValue( ), dstArg.getValue( ));

        bool scale = false;
        double scaleX = 1.0;
        double scaleY = 1.0;
        double scaleZ = 1.0;
        if( sclArg.isSet() ) // scale volume common
        {
            scaleX = scaleY = scaleZ = sclArg.getValue( );
            scale  = true;
        }
        if( sclXArg.isSet() ) // scale volume X
        {
            scaleX = sclXArg.getValue( );
            scale  = true;
        }
        if( sclYArg.isSet() ) // scale volume Y
        {
            scaleY = sclYArg.getValue( );
            scale  = true;
        }
        if( sclZArg.isSet() ) // scale volume Z
        {
            scaleZ = sclZArg.getValue( );
            scale  = true;
        }

        if( scale )
            return RawConverter::ScaleRawDerFile(
                        srcArg.getValue( ), dstArg.getValue( ),
                        scaleX, scaleY, scaleZ                  );


        LBERROR << "Converter options were not specified completely." << endl;
    }
    catch( TCLAP::ArgException& exception )
    {
        LBERROR << " Command line parse error: " << exception.error()
                << " for argument " << exception.argId() << endl;
    }
    return 1;
}


static void getPredefinedHeaderParameters
(
    const string& fileName,
    unsigned &w, unsigned &h, unsigned &d,
    vector<unsigned char> &TF                     );

static void CreateTransferFunc( int t, unsigned char *transfer );


static int calculateAndSaveDerivatives( const string& dst,
                                        unsigned char *volume,
                                        const unsigned w,
                                        const unsigned h,
                                        const unsigned d  );


static int readDimensionsFromSav( FILE*     file,
                                  unsigned& w,
                                  unsigned& h,
                                  unsigned& d     )
{
    if( fscanf( file, "w=%u\n", &w ) == EOF )
        ::exit( EXIT_FAILURE );
    if( fscanf( file, "h=%u\n", &h ) == EOF )
        ::exit( EXIT_FAILURE );
    if( fscanf( file, "d=%u\n", &d ) != 1 )
        return 1;

    return 0;
}


static int writeDimensionsToSav(       FILE*    file,
                                 const unsigned w,
                                 const unsigned h,
                                 const unsigned d       )
{
        fprintf( file, "w=%u\n", w );
        fprintf( file, "h=%u\n", h );
    if( fprintf( file, "d=%u\n", d ) != 1 )
        return 1;

    return 0;
}


static int readScalesFormSav( FILE* file,
                              float& wScale,
                              float& hScale,
                              float& dScale  )
{
    if( fscanf( file, "wScale=%g\n", &wScale ) == EOF )
        ::exit( EXIT_FAILURE );
    if( fscanf( file, "hScale=%g\n", &hScale ) == EOF )
        ::exit( EXIT_FAILURE );
    if( fscanf( file, "dScale=%g\n", &dScale ) != 1 )
        return 1;

    return 0;
}


static int writeScalesToSav( FILE* file,
                             const float wScale,
                             const float hScale,
                             const float dScale  )
{
        fprintf( file, "wScale=%g\n", wScale );
        fprintf( file, "hScale=%g\n", hScale );
    if( fprintf( file, "dScale=%g\n", dScale ) != 1 )
        return 1;

    return 0;
}


static int readTransferFunction( FILE* file,  vector<unsigned char>& TF )
{
    if( fscanf(file,"TF:\n") !=0 )
        return lFailed( "Error in header file", 0 );

    unsigned TFSize;
    if( fscanf( file, "size=%u\n", &TFSize ) == EOF )
        ::exit( EXIT_FAILURE );

    if( TFSize!=256  )
        LBWARN << "Wrong size of transfer function, should be 256" << endl;

    TFSize = clip<int>( TFSize, 1, 256 );

    int tmp;
    for( unsigned i=0; i<TFSize; i++ )
    {
        if( fscanf( file, "r=%d\n", &tmp ) == EOF )
            ::exit( EXIT_FAILURE );
        TF[4*i  ] = tmp;
        if( fscanf( file, "g=%d\n", &tmp ) == EOF )
            ::exit( EXIT_FAILURE );
        TF[4*i+1] = tmp;
        if( fscanf( file, "b=%d\n", &tmp ) == EOF )
            ::exit( EXIT_FAILURE );
        TF[4*i+2] = tmp;
        if( fscanf( file, "a=%d\n", &tmp ) != 1 )
        {
            LBERROR << "Failed to read entity #" << i
                    << " of TF from first header file" << endl;
            return i;
        }
        TF[4*i+3] = tmp;
    }

    return 256;
}


static int writeTransferFunction( FILE* file,  const vector<unsigned char>& TF )
{
    int TFSize = int( TF.size() / 4 );

    fprintf( file,"TF:\n" );

    fprintf( file, "size=%d\n", TFSize );

    int tmp;
    for( int i=0; i<TFSize; i++ )
    {
        tmp = TF[4*i  ]; fprintf( file, "r=%d\n", tmp );
        tmp = TF[4*i+1]; fprintf( file, "g=%d\n", tmp );
        tmp = TF[4*i+2]; fprintf( file, "b=%d\n", tmp );
        tmp = TF[4*i+3]; fprintf( file, "a=%d\n", tmp );
    }

    return 0;
}

static int writeVHF( const string&  filename,
                     const unsigned w,
                     const unsigned h,
                     const unsigned d,
                     const float    wScale,
                     const float    hScale,
                     const float    dScale,
                     const vector<unsigned char>& TF )
{
    hFile info( fopen( filename.c_str(), "wb" ) );
    FILE* file = info.f;

    if( file==NULL ) return lFailed( "Can't open destination header file" );

    writeDimensionsToSav( file, w, h, d );
    writeScalesToSav( file, wScale, hScale, dScale );

    writeTransferFunction( file, TF );

    return 0;
}

int RawConverter::CompareTwoRawDerVhf( const string& src1,
                                       const string& src2 )
{
    LBWARN << "Comparing two raw+derivatives+vhf" << endl;
    unsigned  w1,  h1,  d1;
    unsigned  w2,  h2,  d2;
    float    sw1, sh1, sd1;
    float    sw2, sh2, sd2;

    vector< unsigned char > TF1( 256*4, 0 );
    vector< unsigned char > TF2( 256*4, 0 );

    //reading headers
    string configFileName = src1;
    hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
    FILE* file = info.f;

    if( file==NULL ) return lFailed( "Can't open first header file" );

    //reading dimensions
    readDimensionsFromSav( file,  w1,  h1,  d1 );
    if( readScalesFormSav( file, sw1, sh1, sd1 ) )
        lFailed( "Wrong format of the first header file" );

    //reading transfer function
    const size_t tfSize1 = readTransferFunction( file, TF1 );

    configFileName = src2;
    hFile info2( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
    file = info2.f;

    if( file==NULL ) return lFailed( "Can't open first header file" );

    readDimensionsFromSav( file,  w2,  h2,  d2 );
    if( readScalesFormSav( file, sw2, sh2, sd2 ) )
        lFailed( "Wrong format of the second header file" );

    //reading transfer function
    const size_t tfSize2 = readTransferFunction( file, TF2 );

    //comparing headers
    if( w1!=w2 ) return lFailed(" Widths are not equal ");
    if( h1!=h2 ) return lFailed(" Heights are not equal ");
    if( d1!=d2 ) return lFailed(" Depths are not equal ");

    if( sw1!=sw2 ) LBWARN << " Widths'  scales are not equal " << endl;
    if( sh1!=sh2 ) LBWARN << " Heights' scales are not equal " << endl;
    if( sd1!=sd2 ) LBWARN << " Depths'  scales are not equal " << endl;

    if( tfSize1!=tfSize2 ) LBWARN << " TF sizes are not equal" << endl;

    LBWARN << "done" << endl;
    return 0;
}


int RawConverter::RawPlusDerivativesToRawConverter( const string& src,
                                                    const string& dst )
{
    unsigned w, h, d;
//read header
    {
        string configFileName = src;
        hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open header file" );

        readDimensionsFromSav( file, w, h, d );
    }
    LBWARN << "Dropping derivatives from raw+derivatives model: "
           << src << " " << w << " x " << h << " x " << d << endl;

//read model
    vector<unsigned char> volume( w*h*d*4, 0 );

    LBWARN << "Reading model" << endl;
    {
        ifstream file( src.c_str(),
                       ifstream::in | ifstream::binary | ifstream::ate );

        if( !file.is_open() )
            return lFailed( "Can't open volume file" );

        ifstream::pos_type size;

        size = min( (int)file.tellg(), (int)volume.size() );

        file.seekg( 0, ios::beg );
        file.read( (char*)( &volume[0] ), size );

        file.close();
    }
//remove derivatives
    {
        unsigned char* sr = &volume[3];
        unsigned char* ds = &volume[0];
        for( unsigned i = 3; i < w*h*d*4; i+=4 )
        {
            *ds = *sr;
            sr += 4;
            ++ds;
        }
        volume.resize( w*h*d );
    }

//save file
    {
       ofstream file( dst.c_str(),
                       ifstream::out | ifstream::binary | ifstream::trunc );

       if( !file.is_open() )
           return lFailed( "Can't open destination volume file" );

        file.write( (char*)( &volume[0] ), volume.size() );

        file.close();
    }
    LBWARN << "done" << endl;
    return 0;
}


int RawConverter::RawToRawPlusDerivativesConverter( const string& src,
                                                    const string& dst )
{
    unsigned w, h, d;
//read header
    {
        string configFileName = src;
        hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open header file" );

        readDimensionsFromSav( file, w, h, d );
    }
    LBWARN << "Creating derivatives for raw model: "
           << src << " " << w << " x " << h << " x " << d << endl;

//read model
    vector<unsigned char> volume( w*h*d, 0 );

    LBWARN << "Reading model" << endl;
    {
        ifstream file( src.c_str(),
                       ifstream::in | ifstream::binary | ifstream::ate );

        if( !file.is_open() )
            return lFailed( "Can't open volume file" );

        ifstream::pos_type size;

        size = min( (int)file.tellg(), (int)volume.size() );

        file.seekg( 0, ios::beg );
        file.read( (char*)( &volume[0] ), size );

        file.close();
    }

//calculate and save derivatives
    {
        int result = calculateAndSaveDerivatives( dst, &volume[0], w,  h, d );

        if( result ) return result;
    }
    LBWARN << "done" << endl;
    return 0;
}


int RawConverter::RecalculateDerivatives( const string& src,
                                          const string& dst )
{
    unsigned w, h, d;
//read header
    {
        string configFileName = src;
        hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open header file" );

        readDimensionsFromSav( file, w, h, d );
    }
    LBWARN << "Creating derivatives for raw model: "
           << src << " " << w << " x " << h << " x " << d << endl;

//read model
    vector<unsigned char> volume( w*h*d*4, 0 );

    LBWARN << "Reading model" << endl;
    {
        ifstream file( src.c_str(),
                       ifstream::in | ifstream::binary | ifstream::ate );

        if( !file.is_open() )
            return lFailed( "Can't open volume file" );

        ifstream::pos_type size;

        size = min( (int)file.tellg(), (int)volume.size() );

        file.seekg( 0, ios::beg );
        file.read( (char*)( &volume[0] ), size );

        file.close();
    }
//remove derivatives
    {
        unsigned char* sr = &volume[3];
        unsigned char* ds = &volume[0];
        for( unsigned i = 3; i < w*h*d*4; i+=4 )
        {
            *ds = *sr;
            sr += 4;
            ++ds;
        }
        volume.resize( w*h*d );
    }

//calculate and save derivatives
    {
        int result = calculateAndSaveDerivatives( dst, &volume[0], w, h, d );

        if( result ) return result;
    }
    LBWARN << "done" << endl;
    return 0;
}


int RawConverter::SavToVhfConverter( const string& src, const string& dst )
{
    //read original header
    unsigned w=1;
    unsigned h=1;
    unsigned d=1;
    float wScale=1.0;
    float hScale=1.0;
    float dScale=1.0;
    {
        hFile info( fopen( dst.c_str(), "rb" ) );
        FILE* file = info.f;

        if( file!=NULL )
        {
            readDimensionsFromSav( file, w, h, d );
            readScalesFormSav( file, wScale, hScale, dScale );
        }
    }

    //read sav
    int TFSize = 256;
    vector< unsigned char > TF( 256*4, 0 );

    {
        hFile info( fopen( src.c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL )
        {
            LBWARN << "Can't open source sav file." << endl
                   << "Using predefined transfer functions and parameters."
                   << endl;

            getPredefinedHeaderParameters( src, w, h, d, TF );
        }else
        {
            float t;
            int   ti;
            float tra;
            float tga;
            float tba;
            if( fscanf( file, "2DTF:\n"          ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "num=%d\n"    , &ti) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "mode=%d\n"   , &ti) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "rescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "gescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "bescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "rascale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "gascale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "bascale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "TF:\n"            ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "res=%d\n",&TFSize ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "rescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "gescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "bescale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "rascale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "gascale=%f\n", &t ) == EOF )
                ::exit( EXIT_FAILURE );
            if( fscanf( file, "bascale=%f\n", &t ) != 1)
                return lFailed( "failed to read header of sav file" );

            if( TFSize!=256  )
                return lFailed( "Wrong size of transfer function, != 256" );

            TF.resize( TFSize*4 );

            for( int i=0; i<TFSize; i++ )
            {
                if( fscanf( file, "re=%f\n", &t   ) == EOF )
                    ::exit( EXIT_FAILURE );
                TF[4*i  ] = clip( static_cast<int>( t*255.0 ), 0, 255 );

                if( fscanf( file, "ge=%f\n", &t   ) == EOF )
                    ::exit( EXIT_FAILURE );
                TF[4*i+1] = clip( static_cast<int>( t*255.0 ), 0, 255 );

                if( fscanf( file, "be=%f\n", &t   ) == EOF )
                    ::exit( EXIT_FAILURE );
                TF[4*i+2] = clip( static_cast<int>( t*255.0 ), 0, 255 );

                if( fscanf( file, "ra=%f\n", &tra ) == EOF )
                    ::exit( EXIT_FAILURE );
                if( fscanf( file, "ga=%f\n", &tba ) == EOF )
                    ::exit( EXIT_FAILURE );
                if( fscanf( file, "ba=%f\n", &tga ) !=1 )
                {
                    LBERROR << "Failed to read entity #"
                            << i << " of sav file" << endl;
                    return 1;
                }
                TF[4*i+3] =
                    clip( static_cast<int>((tra+tga+tba)*255.0/3.0 ), 0, 255 );
            }
        }
    }

    //write vhf
    int result = writeVHF( dst, w, h, d, wScale, hScale, dScale, TF );
    if( result ) return result;

    LBWARN << "file " << src.c_str() << " > " << dst.c_str()
           << " converted" << endl;

    return 0;
}


int RawConverter::DscToVhfConverter( const string& src, const string& dst )
{
    LBWARN << "converting " << src.c_str() << " > " << dst.c_str() << " .. ";

    //Read Description file
    unsigned w=1;
    unsigned h=1;
    unsigned d=1;
    float wScale=1.0;
    float hScale=1.0;
    float dScale=1.0;
    {
        hFile info( fopen( src.c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open source Dsc file" );

        if( fscanf( file, "reading PVM file\n" ) != 0 )
            return lFailed( "Not a proper file format, \
                             first line should be:\nreading PVM file" );

        unsigned c=0;
        if( fscanf( file,
            "found volume with width=%u height=%u depth=%u components=%u\n",
            &w, &h, &d, &c ) != 4 )
            return lFailed( "Not a proper file format, second line should \
                             be:\nfound volume with width=<num> height=<num>  \
                             depth=<num> components=<num>" );
        if( c!=1 )
            return lFailed( "'components' should be equal to '1', \
                             only 8 bit volumes supported so far" );

        if( fscanf( file, "and edge length %g/%g/%g\n",
                    &wScale, &hScale, &dScale ) == EOF )
            ::exit( EXIT_FAILURE );
    }
    //Write Vhf file
    {
        hFile info( fopen( dst.c_str(), "wb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open destination header file" );

        writeDimensionsToSav( file, w, h, d );
        writeScalesToSav( file, wScale, hScale, dScale );
    }
    LBWARN << "succeed" << endl;
    return 0;
}


int RawConverter::PvmSavToRawDerVhfConverter(     const string& src,
                                                  const string& dst  )
{
    LBWARN << "Converting " << src << " -> " << dst << endl;

    // reading pvm volume
    unsigned char*  volume = 0;
    unsigned        width,  height, depth, components;
    float           scaleX, scaleY, scaleZ;
    char*           srcTmp = const_cast<char*>( src.c_str() );

    volume = readPVMvolume( srcTmp,  &width,  &height, &depth,
                                &components, &scaleX, &scaleY, &scaleZ  );

    if( volume==NULL )
        return lFailed( "Can't read vpm file" );

    if( components != 1 )
        return lFailed( "The only 8-bits models are supported" );

    LBWARN  << "dimensions: "
            << width << " x " << height << " x " << depth << endl
            << "scales: " << scaleX << " x " << scaleY << " x " << scaleZ
            << endl;

    // calculating derivatives
    int result =
        calculateAndSaveDerivatives( dst, volume, width,  height, depth );

    free( volume );
    if( result ) return result;

    //converting transfer function
    SavToVhfConverter( src+".sav", dst+".vhf" );

    LBWARN << "done" << endl;

    return 0;
}


int RawConverter::ScaleRawDerFile(                  const string& src,
                                                    const string& dst,
                                                          double scaleX,
                                                          double scaleY,
                                                          double scaleZ  )
{
    LBWARN << "scaleW: " << scaleX << endl;
    LBWARN << "scaleH: " << scaleY << endl;
    LBWARN << "scaleD: " << scaleZ << endl;

    if( scaleX < 0.0001 ) lFailed( "Scale for width  is too small" );
    if( scaleY < 0.0001 ) lFailed( "Scale for height is too small" );
    if( scaleZ < 0.0001 ) lFailed( "Scale for depth  is too small" );

    LBWARN << "Scaling raw+derivatives+vhf" << endl;
    unsigned wS, hS, dS;
    float    sw, sh, sd;

    vector< unsigned char > TF( 256*4, 0 );
    //reading header
    {
        string configFileName = src;
        hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open source header file" );

        //reading dimensions
        readDimensionsFromSav( file, wS, hS, dS );
        if( readScalesFormSav( file, sw, sh, sd ) )
            lFailed( "Wrong format of the source header file" );

        //reading transfer function
        readTransferFunction( file, TF );
    }
    //writing header
    unsigned wD = static_cast<unsigned>( wS*scaleX );
    unsigned hD = static_cast<unsigned>( hS*scaleY );
    unsigned dD = static_cast<unsigned>( dS*scaleZ );
    {
        string vhf = dst;
        int result = writeVHF( vhf.append( ".vhf" ), wD, hD, dD,
                                                     sw, sh, sd, TF );
        if( result ) return result;
    }
    LBWARN << "old dimensions: " << wS << " x " << hS << " x " << dS << endl;
    LBWARN << "new dimensions: " << wD << " x " << hD << " x " << dD << endl;

    //read volume
    vector<unsigned char> sVol( wS*hS*dS*4, 0 );

    LBWARN << "Reading model" << endl;
    {
        ifstream file( src.c_str(),
                       ifstream::in | ifstream::binary | ifstream::ate );

        if( !file.is_open() )
            return lFailed( "Can't open volume file" );

        ifstream::pos_type size;

        size = min( (int)file.tellg(), (int)sVol.size() );

        file.seekg( 0, ios::beg );
        file.read( (char*)( &sVol[0] ), size );

        file.close();
    }
    LBWARN << "Scaling model" << endl;
    //scale volume
    vector<unsigned char> dVol( wD*hD*dD*4, 0 );
    {
        int wD4   = wD*4;
        int wDhD4 = wD*hD*4;
        int wS4   = wS*4;
        int wShS4 = wS*hS*4;

        int scaleIx  = static_cast<int>( scaleX );
        int scaleIy  = static_cast<int>( scaleY );
        int scaleIz  = static_cast<int>( scaleZ );
        int tenPerc  = (dD-scaleIz-1) / 10;
        int tenPercI = 0;
        for( unsigned z=0; z<dD-scaleIz; z++ )
        {
            if( ++tenPercI >= tenPerc )
            {
                std::cout << ".";
                std::cout.flush();
                tenPercI = 0;
            }
            for( unsigned y=0; y<hD-scaleIy; y++ )
                for( unsigned x=0; x<wD-scaleIx; x++ )
                {
                    double cx = x/scaleX;
                    double cy = y/scaleY;
                    double cz = z/scaleZ;

                    int nx = static_cast<int>( cx );
                    int ny = static_cast<int>( cy );
                    int nz = static_cast<int>( cz );

                    int fx = nx+1;
                    int fy = ny+1;
                    int fz = nz+1;

                    cx -= nx;
                    cy -= ny;
                    cz -= nz;

                    double v1 = (1-cx)*(1-cy)*(1-cz);
                    double v2 =    cx *(1-cy)*(1-cz);
                    double v3 = (1-cx)*(1-cy)*   cz;
                    double v4 =    cx *(1-cy)*   cz;
                    double v5 = (1-cx)*   cy *(1-cz);
                    double v6 =    cx *   cy *(1-cz);
                    double v7 = (1-cx)*   cy *   cz ;
                    double v8 =    cx *   cy *   cz ;

                    int p1 = nx*4 + ny*wS4 + nz*wShS4;
                    int p2 = fx*4 + ny*wS4 + nz*wShS4;
                    int p3 = nx*4 + ny*wS4 + fz*wShS4;
                    int p4 = fx*4 + ny*wS4 + fz*wShS4;
                    int p5 = nx*4 + fy*wS4 + nz*wShS4;
                    int p6 = fx*4 + fy*wS4 + nz*wShS4;
                    int p7 = nx*4 + fy*wS4 + fz*wShS4;
                    int p8 = fx*4 + fy*wS4 + fz*wShS4;

                    int pD =  x*4 +  y*wD4 +  z*wDhD4;

                    for( int d = 0; d<4; d++)
                    {
                        double res = v1*sVol[p1+d] + v2*sVol[p2+d] +
                                     v3*sVol[p3+d] + v4*sVol[p4+d] +
                                     v5*sVol[p5+d] + v6*sVol[p6+d] +
                                     v7*sVol[p7+d] + v8*sVol[p8+d];

                        dVol[pD+d] = min<int>( static_cast<int>( res ), 255 );
                    }
                }
        }
        std::cout << endl;
    }

    //write new volume
    LBWARN << "Writing model" << endl;
    {
        ofstream file ( dst.c_str(),
                        ifstream::out | ifstream::binary | ifstream::trunc );

        if( !file.is_open() )
            return lFailed( "Can't open destination volume file" );

        file.write( (char*)( &dVol[0] ), dVol.size() );
        file.close();
    }

    LBWARN << "Done" << endl;
    return 0;
}


static int calculateAndSaveDerivatives( const string& dst,
                                        unsigned char *volume,
                                        const unsigned w,
                                        const unsigned h,
                                        const unsigned d  )
{
    LBWARN << "Calculating derivatives" << endl;
    ofstream file ( dst.c_str(),
                    ifstream::out | ifstream::binary | ifstream::trunc );

    if( !file.is_open() )
        return lFailed( "Can't open destination volume file" );

    const int wh = w*h;

    const int ws = static_cast<int>( w );

    vector<unsigned char> GxGyGzA( wh*d*4, 0 );

    for( unsigned z=1; z<d-1; z++ )
    {
        const int zwh = z*wh;

        const unsigned char *curPz = &volume[0] + zwh;

        for( unsigned y=1; y<h-1; y++ )
        {
            const int zwh_y = zwh + y*w;
            const unsigned char * curPy = curPz + y*w;
            for( unsigned x=1; x<w-1; x++ )
            {
                const unsigned char * curP = curPy +  x;
                const unsigned char * prvP = curP  - wh;
                const unsigned char * nxtP = curP  + wh;
                int gx =
                      nxtP[  ws+1 ]+ 3*curP[  ws+1 ]+   prvP[  ws+1 ]+
                    3*nxtP[     1 ]+ 6*curP[     1 ]+ 3*prvP[     1 ]+
                      nxtP[ -ws+1 ]+ 3*curP[ -ws+1 ]+   prvP[ -ws+1 ]-

                      nxtP[  ws-1 ]- 3*curP[  ws-1 ]-   prvP[  ws-1 ]-
                    3*nxtP[    -1 ]- 6*curP[    -1 ]- 3*prvP[    -1 ]-
                      nxtP[ -ws-1 ]- 3*curP[ -ws-1 ]-   prvP[ -ws-1 ];

                int gy =
                      nxtP[  ws+1 ]+ 3*curP[  ws+1 ]+   prvP[  ws+1 ]+
                    3*nxtP[  ws   ]+ 6*curP[  ws   ]+ 3*prvP[  ws   ]+
                      nxtP[  ws-1 ]+ 3*curP[  ws-1 ]+   prvP[  ws-1 ]-

                      nxtP[ -ws+1 ]- 3*curP[ -ws+1 ]-   prvP[ -ws+1 ]-
                    3*nxtP[ -ws   ]- 6*curP[ -ws   ]- 3*prvP[ -ws   ]-
                      nxtP[ -ws-1 ]- 3*curP[ -ws-1 ]-   prvP[ -ws-1 ];

                int gz =
                      nxtP[  ws+1 ]+ 3*nxtP[    1 ]+   nxtP[ -ws+1 ]+
                    3*nxtP[  ws   ]+ 6*nxtP[    0 ]+ 3*nxtP[ -ws   ]+
                      nxtP[  ws-1 ]+ 3*nxtP[   -1 ]+   nxtP[ -ws-1 ]-

                      prvP[  ws+1 ]- 3*prvP[    1 ]-   prvP[ -ws+1 ]-
                    3*prvP[  ws   ]- 6*prvP[    0 ]- 3*prvP[ -ws   ]-
                      prvP[  ws-1 ]- 3*prvP[   -1 ]-   prvP[ -ws-1 ];

                int length = static_cast<int>(
                                        sqrt(double((gx*gx+gy*gy+gz*gz))+1));

                gx = ( gx*255/length + 255 )/2;
                gy = ( gy*255/length + 255 )/2;
                gz = ( gz*255/length + 255 )/2;

                GxGyGzA[(zwh_y + x)*4   ] = static_cast<unsigned char>( gx );
                GxGyGzA[(zwh_y + x)*4 +1] = static_cast<unsigned char>( gy );
                GxGyGzA[(zwh_y + x)*4 +2] = static_cast<unsigned char>( gz );
                GxGyGzA[(zwh_y + x)*4 +3] = curP[0];
            }
        }
    }

    LBWARN << "Writing derivatives: "
           << dst.c_str() << " " << GxGyGzA.size() << " bytes" <<endl;

    file.write( (char*)( &GxGyGzA[0] ), GxGyGzA.size() );

    file.close();

    return 0;
}


static void getPredefinedHeaderParameters( const string& fileName,
                                          unsigned &w, unsigned &h, unsigned &d,
                                          vector<unsigned char> &TF )
{
    int t=w=h=d=0;

    if( fileName.find( "spheres128x128x128"    , 0 ) != string::npos )
        t=0, w=h=d=128;

    if( fileName.find( "fuel"                  , 0 ) != string::npos )
        t=1, w=h=d=64;

    if( fileName.find( "neghip"                , 0 ) != string::npos )
        t=2, w=h=d=64;

    if( fileName.find( "Bucky32x32x32"         , 0 ) != string::npos )
        t=3, w=h=d=32;

    if( fileName.find( "hydrogen"              , 0 ) != string::npos )
        t=4, w=h=d=128;

    if( fileName.find( "Engine256x256x256"     , 0 ) != string::npos )
        t=5, w=h=d=256;

    if( fileName.find( "skull"                 , 0 ) != string::npos )
        t=6, w=h=d=256;

    if( fileName.find( "vertebra8"             , 0 ) != string::npos )
        t=7, w=h=512, d=256;

    if( w==0 )
        w=h=d=8;

    return CreateTransferFunc( t, &TF[0] );
}


static void CreateTransferFunc( int t, unsigned char *transfer )
{
    memset( transfer, 0, 256*4 );

    int i;
    switch(t)
    {
        case 0:  //spheres

            LBWARN << "transfer: spheres" << endl;
            for (i=40; i<255; i++) {
                transfer[(i*4)]   = 115;
                transfer[(i*4)+1] = 186;
                transfer[(i*4)+2] = 108;
                transfer[(i*4)+3] = 255;
            }
            break;

        case 1:// fuel

            LBWARN << "transfer: fuel" << endl;
            for (i=0; i<65; i++) {
                transfer[(i*4)] = 255;
                transfer[(i*4)+1] = 255;
                transfer[(i*4)+2] = 255;
            }
            for (i=65; i<193; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 160;
                transfer[(i*4)+2] = transfer[(192-i)*4];
            }
            for (i=193; i<255; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 255;
            }

            for (i=2; i<80; i++) {
                transfer[(i*4)+3] = (unsigned char)((i-2)*56/(80-2));
            }
            for (i=80; i<255; i++) {
                transfer[(i*4)+3] = 128;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;

        case 2: //neghip

            LBWARN << "transfer: neghip" << endl;
            for (i=0; i<65; i++) {
                transfer[(i*4)] = 255;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 0;
            }
            for (i=65; i<193; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 160;
                transfer[(i*4)+2] = transfer[(192-i)*4];
            }
            for (i=193; i<255; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 255;
            }

            for (i=2; i<80; i++) {
                transfer[(i*4)+3] = (unsigned char)((i-2)*36/(80-2));
            }
            for (i=80; i<255; i++) {
                transfer[(i*4)+3] = 128;
            }
            break;

        case 3: //bucky

            LBWARN << "transfer: bucky" << endl;
            for (i=20; i<99; i++) {
                transfer[(i*4)]  =  200;
                transfer[(i*4)+1] = 200;
                transfer[(i*4)+2] = 200;
            }
            for (i=20; i<49; i++) {
                transfer[(i*4)+3] = 10;
            }
            for (i=50; i<99; i++) {
                transfer[(i*4)+3] = 20;
            }

            for (i=100; i<255; i++) {
                transfer[(i*4)]  =  93;
                transfer[(i*4)+1] = 163;
                transfer[(i*4)+2] = 80;
            }

            for (i=100; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;

        case 4: //hydrogen

            LBWARN << "transfer: hydrogen" << endl;
            for (i=4; i<20; i++) {
                transfer[(i*4)] = 137;
                transfer[(i*4)+1] = 187;
                transfer[(i*4)+2] = 188;
                transfer[(i*4)+3] = 5;
            }

            for (i=20; i<255; i++) {
                transfer[(i*4)]  =  115;
                transfer[(i*4)+1] = 186;
                transfer[(i*4)+2] = 108;
                transfer[(i*4)+3] = 250;
            }
            break;

        case 5: //engine

            LBWARN << "transfer: engine" << endl;
            for (i=100; i<200; i++) {
                transfer[(i*4)] =     44;
                transfer[(i*4)+1] = 44;
                transfer[(i*4)+2] = 44;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)]  =  173;
                transfer[(i*4)+1] = 22;
                transfer[(i*4)+2] = 22;
            }
            // opacity

            for (i=100; i<200; i++) {
                transfer[(i*4)+3] = 8;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;

        case 6: //skull

            LBWARN << "transfer: skull" << endl;
            for (i=40; i<255; i++) {
                transfer[(i*4)]  =  128;
                transfer[(i*4)+1] = 128;
                transfer[(i*4)+2] = 128;
                transfer[(i*4)+3] = 128;
            }
            break;

        case 7: //vertebra

            LBWARN << "transfer: vertebra" << endl;
            for (i=40; i<255; i++) {
                int k = i*2;
                if (k > 255) k = 255;
                transfer[(i*4)]  =  k;
                transfer[(i*4)+1] = k;
                transfer[(i*4)+2] = k;
                transfer[(i*4)+3] = k;
            }
            break;

        default:
        LBWARN << "transfer: linear (default)" << endl;
            for (i=0; i<255; i++) {
                transfer[(i*4)]   = i;
                transfer[(i*4)+1] = i;
                transfer[(i*4)+2] = i;
                transfer[(i*4)+3] = i;
            }
    }
}


}


