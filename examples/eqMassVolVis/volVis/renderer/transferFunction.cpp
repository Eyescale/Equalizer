
#include "transferFunction.h"
#include <msv/util/fileIO.h>
#include <msv/util/hlp.h>
#include <msv/util/debug.h>

#include <fstream>

namespace massVolVis
{

TransferFunction::TransferFunction()
{
    rgba.resize( 256*4 );
    sda.resize(  256*3 );
}

void TransferFunction::initDefault()
{
    const int th1 = 30*4;
    const int th2 = 80*4;
    for( int i = 0; i < 256*4; i+=4 )
    {
        const byte val = ( i < th1 ) ? 0 : ( i > th2 ? 255 : (i-th1)*255/(th2-th1));
        rgba[i+1] = rgba[i+3] = val;
        rgba[i]   = rgba[i+2] = val;
    }
    for( int i = 0; i < 256*3; i+=3 )
    {
        sda[i] = sda[i+1] = sda[i+2] = 255;
    }
}


bool TransferFunction::load( const std::string& fName )
{
    LOG_INFO << "Loading TF: " << fName.c_str() << std::endl;

    if( util::fileSize( fName ) == 0 )
        return false;

    std::ifstream is;
    is.open( fName.c_str(), std::ios_base::in );
    if( !is.is_open( ))
    {
        LOG_ERROR << "Can't open file to read: " << fName.c_str() << std::endl;
        return false;
    }

    is.seekg( 0, std::ios::beg );
    if( is.tellg() != 0 )
    {
        LOG_ERROR << "Can't proceed to the offset: " << 0 << " to read file: " << fName.c_str() << std::endl;
        is.close();
        return false;
    }

    // convert float TF valus to native format
    memset( &rgba[0], 0, rgba.size()*sizeof(rgba[0]) );
    memset( &sda[0],  0,  sda.size()*sizeof( sda[0]) );

    size_t rgbaPos = 0;
    size_t  sdaPos = 0;
    for( size_t i = 0; i < 256; ++i )
    {
        float v[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        for( int j = 0; j < 7; ++j ) // rgba
        {
            if( is.good() )
                is >> v[j];
            else
            {
                i = 256;
                break;
            }
        }
        for( int j = 0; j < 4; ++j ) // rgba
            rgba[rgbaPos++] = static_cast<byte>( hlpFuncs::myClip( v[j  ]*255.f, 0.f, 255.f ));

        for( int j = 0; j < 3; ++j ) // sda
            sda[  sdaPos++] = static_cast<byte>( hlpFuncs::myClip( v[j+4]*255.f, 0.f, 255.f ));
    }

    if( is.fail( ))
        LOG_ERROR << "Some error happen during reading of a file: " << fName.c_str() << std::endl;

    is.close();

    return true;
}


co::DataOStream& operator << ( co::DataOStream& os, const TransferFunction& tf )
{
    os << tf.rgba;
    os << tf.sda;

    return os;
}


co::DataIStream& operator >> ( co::DataIStream& is, TransferFunction& tf )
{
    is >> tf.rgba;
    is >> tf.sda;

    return is;
}


} //namespace massVolVis
