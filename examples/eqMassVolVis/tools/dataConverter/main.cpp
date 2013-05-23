
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "dataConverter.h"
#include "dataManipulation.h"

//#include <msv/tree/volumeTree.h>

#include <msv/IO/dataHDDIORaw.h>
#include <msv/IO/dataHDDIOOctree.h>
#include <msv/util/debug.h>
#include <msv/util/hlp.h>

#include <stdio.h>

#include <tclap/CmdLine.h>

#include <math.h>

using namespace massVolVis;

#if 1
int main( int argc, char **argv )
{
    VolumeFileInfo fileInfo;

    VolumeFileInfo srcFileInfo;
    VolumeFileInfo dstFileInfo;
    try
    {
        TCLAP::CmdLine command( DataConverter::getHelp( ));

        TCLAP::ValueArg<std::string> srcDimensionsArg( "w", "srcDim",
                                             "source dimensions: \"64,128,128\" or \"64x128x128\"",
                                               false, "", "string", command );

        TCLAP::ValueArg<uint> octreeBSArg( "k", "octDim", "Octree block size",
                                               false, 64, "integer", command );

        TCLAP::ValueArg<int> dstBitsArg( "n", "outBits", "number of bits in the output file (8, 16)",
                                       false, 8, "integer", command );

        TCLAP::ValueArg<int> srcBitsArg( "b", "inBits", "number of bits in the input file (8, 16)",
                                       false, 8, "integer", command );

        TCLAP::ValueArg<uint> borderArg( "e", "border", "number of border elements (width)",
                                       false, 0, "integer", command );

        TCLAP::ValueArg<std::string> dstTypeArg( "o", "outType", "dst file type (oct, oct_head)",
                                       false, "oct_head", "string", command );

        TCLAP::ValueArg<std::string> srcTypeArg( "i", "inType", "src file type (raw, raw_der, oct)",
                                       false, "raw_der", "string", command );

        TCLAP::ValueArg<std::string> dstArg( "d", "dst", "destination file / folder",
                                       false, "", "string", command );

        TCLAP::ValueArg<std::string> srcArg( "s", "src", "source file / folder",
                                       false, "", "string", command );

        command.parse( argc, argv );

        DataConverter dataConverter;

// source and destination have to be set
        if( !srcArg.isSet( ))
            throw( TCLAP::ArgException( "source file is missing", "src" ));

        if( !dstArg.isSet( ))
            throw( TCLAP::ArgException( "destination file is missing", "dst" ));

// octree size has to be power of 2
        if( octreeBSArg.getValue() != hlpFuncs::getPow2( octreeBSArg.getValue( )))
            throw( TCLAP::ArgException( "octree block size has to be power of 2", "octDim" ));

// convert source dimentsions from string to int values
        Vec3_ui32 srcDim;
        if( srcDimensionsArg.isSet( ))
        {
            std::stringstream ss( srcDimensionsArg.getValue( ));
            ss >> srcDim.x; ss.ignore( 1 );
            ss >> srcDim.y; ss.ignore( 1 );
            ss >> srcDim.z;
        }

// source and destination bit depth have to be 8 or 16
        if( srcBitsArg.getValue() !=  8 &&
            srcBitsArg.getValue() != 16    )
                throw( TCLAP::ArgException( "source bits value has to be 8 or 16", "inBits" ));

        if( dstBitsArg.getValue() !=  8 &&
            dstBitsArg.getValue() != 16    )
                throw( TCLAP::ArgException( "source bits value has to be 8 or 16", "outBits" ));

        if( srcBitsArg.getValue() != dstBitsArg.getValue() )
                throw( TCLAP::ArgException( "source bits have to match destination bits", "outBits" ));


// source type has to be "raw", "raw_der" or "oct"
        if( srcTypeArg.getValue() == "raw" )
        {
            srcFileInfo.setBytesNum( srcBitsArg.getValue() / 8 );
            LOG_INFO << "in bits: " << srcBitsArg.getValue() << " in bytes:" << srcFileInfo.getBytesNum() << std::endl;
            dataConverter.setReaderType( DataConverter::RAW );
        }
        else if( srcTypeArg.getValue() == "raw_der" )
        {
            if( srcBitsArg.getValue() != 8 )
                throw( TCLAP::ArgException( "only 8 bits raw_der is supported", "inBits" ));

            srcFileInfo.setBytesNum( 4 ); // raw + 3x derivatives

            dataConverter.setReaderType( DataConverter::RAW_DER );
        }
        else if( srcTypeArg.getValue() == "oct" )
        {
            srcFileInfo.setBytesNum( srcBitsArg.getValue() / 8 );

            dataConverter.setReaderType( DataConverter::OCT );
            srcDim = Vec3_ui32( octreeBSArg.getValue( ));
        }else
            throw( TCLAP::ArgException( "srcType is wrong", "inType" ));

        // dimentions have to be set
        if( !( srcDim > Vec3_ui32( 0 )))
        {
            LOG_ERROR << "wrong dimensions: " << srcDim << std::endl;
            throw( TCLAP::ArgException( "dimensions should be positive", "srcDim" ));
        }

        srcFileInfo.setSourceDims( srcDim );
        srcFileInfo.setDataFileName( srcArg.getValue( ));

// source initialization is finished, set it in the data converter
        dataConverter.setDataReader( &srcFileInfo );

// destination type has to be "oct" or "oct_head"
        if( dstTypeArg.getValue() == "oct" )
        {
            dataConverter.setWriterType( DataConverter::OCT );
        }else if( dstTypeArg.getValue() == "oct_head" )
        {
            dataConverter.setWriterType( DataConverter::OCT_HEAD );
        }else
            throw( TCLAP::ArgException( "dstType is wrong", "outType" ));

        dstFileInfo.setBytesNum( dstBitsArg.getValue() / 8 );
        dstFileInfo.setBlockDim( octreeBSArg.getValue( ));
        dstFileInfo.setDataFileName( dstArg.getValue( ));

        if( borderArg.isSet() )
        {
            if( borderArg.getValue() < 256 )
            {
                srcFileInfo.setBorderDim( borderArg.getValue());
                dstFileInfo.setBorderDim( borderArg.getValue());
            }
            else
                LOG_INFO << "Wrong border size" << std::endl;
        }

// destination initialization is finished, set it in the data converter and convert
        dataConverter.setDataWriter( &dstFileInfo );

        LOG_INFO << "-----------------------"                   << NL
                 << "src name: " << srcArg.getValue()           << NL
                 << "    type: " << srcTypeArg.getValue()       << NL
                 << "    bits: " << srcFileInfo.getBytesNum()*8 << NL
                 << "    dim:  " << srcDim                      << NL
                 << "-----------------------"                   << NL
                 << "dst name: " << dstArg.getValue()           << NL
                 << "    type: " << dstTypeArg.getValue()       << NL
                 << "    bits: " << dstFileInfo.getBytesNum()*8 << NL
                 << "      BS: " << octreeBSArg.getValue()      << NL
                 << "    border: " << static_cast< int >(dstFileInfo.getBorderDim()) << NL
                 << "-----------------------"                                        << std::endl;

        dataConverter.convert();
    }
    catch( TCLAP::ArgException& exception )
    {
        LOG_ERROR   << "Command line parse error: " << exception.error()
                    << " for argument " << exception.argId() << std::endl;

    }
    catch( char* e )
    {
        LOG_ERROR << "Error occure: " << e << std::endl;
    }

    return 0;
}
#endif

#if 0
int main( int, char** )
{
    DataHDDIORaw in;
    in.setBytesSize( 2 );
    in.setInName( "/Volumes/Black/maxus/Models/CT/27092010_microct_raw/ct_16bit_2600x1600x5196.raw" );
    in.setSize( Vec3_ui32( 2600, 1600, 5196 ));

    DataHDDIOOctree out;
    out.setBytesSize( 2 );
    out.setOutName( "/Users/maxus/tmp" );
    out.setSize( Vec3_ui32( 256 ));

    byte* data = new byte[ 256*256*256*2 ];

    Vec3_i32 s( 1540, 410, 4618 );

    in.read( Box_i32( s, s+256 ), data );
    out.write( 1, data );

    delete [] data;
    return 0;
}
#endif


#if 0

    const massVolVis::Vec3_ui32 size( 512, 512, 512 );

/*    massVolVis::DataHDDIOOctree octreeData;
    octreeData.setInName(  "/Users/maxus/tmp" );
    octreeData.setOutName( "/Users/maxus/tmp" );
    octreeData.setSize( size );
    octreeData.setBytesSize( 2 );
*/
    byte* buf  = new byte[size.x*size.y*size.z*2];
    byte* buf2 = new byte[size.x*size.y*size.z*2];

    LOG_INFO << "{" << std::endl;
    massVolVis::DataManipulation::reduceTwice( buf2, buf, size, 2 );
    LOG_INFO << "}" << std::endl;

/*
    octreeData.write(    0, buf );
    octreeData.write( 4095, buf );
    octreeData.write( 4096, buf );
    octreeData.write( 8191, buf );
    octreeData.write( 8192, buf );
*/
    delete []buf;
    delete []buf2;

    massVolVis::VolumeTree tree( 64300, 65000, 65000, 512 );
//    massVolVis::VolumeTree tree( 65535, 65535, 65535, 512 );
//    massVolVis::VolumeTree tree( 2600, 1600, 5200, 128 );

//    massVolVis::VolumeTree tree( 512, 512, 512, 256 );

    std::string s = "/Users/maxus/Documents/projects/vmml/_VolVis/largeVolVis/treeVisualizer/data.txt";
    tree.saveTree( s );
    tree.loadTree( s );
#endif

