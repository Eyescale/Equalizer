
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#define EQ_TEST_RUNTIME 120 // seconds
#include <test.h>

#include <eq/init.h>
#include <eq/nodeFactory.h>

#include <co/base/buffer.h>
#include <co/base/clock.h>
#include <co/base/file.h>
#include <co/base/global.h>
#include <co/base/memoryMap.h>
#include <co/base/pluginRegistry.h>
#include <co/base/types.h>

#include <iostream>  // for std::cerr
#include <numeric>
#include <fstream>
#include <sstream>
#include <string>

#include "libs/collage/base/compressorInfo.h" // private header
#include "libs/collage/base/cpuCompressor.h" // private header
#include "libs/collage/base/plugin.h" // private header

void testCompressByte( const uint32_t nameCompressor,
                       const uint8_t* data, const uint64_t size,
                       uint64_t& _compressedSize,
                       float& _timeCompress, 
                       float& _timeDecompress );

void testCompressorFile( );
std::vector< uint32_t > getCompressorNames( const uint32_t tokenType );
void compare( const uint8_t *dst, const uint8_t *src, const uint32_t nbytes );

std::vector< std::string > getFiles( const std::string path, 
                                     std::vector< std::string >& files, 
                                     const std::string& ext );

co::base::Bufferb* readDataFile( const std::string& filename );

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    eq::init( argc, argv, &nodeFactory );
    testCompressorFile();
    eq::exit();

    return EXIT_SUCCESS;
}

std::vector< uint32_t > getCompressorNames( const uint32_t tokenType )
{
    co::base::PluginRegistry& registry = co::base::Global::getPluginRegistry();
    const co::base::Plugins& plugins = registry.getPlugins();

    std::vector< uint32_t > names;
    for( co::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const co::base::CompressorInfos& infos = (*i)->getInfos();
        for( co::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            if ( (*j).tokenType == tokenType )
                names.push_back( (*j).name );
        }
    }
    
    return names;
}

void testCompressByte( const uint32_t nameCompressor,
                       uint8_t* data, const uint64_t size,
                       uint64_t& _compressedSize,
                       float& _timeCompress, 
                       float& _timeDecompress )
{
    co::base::CPUCompressor compressor;
    co::base::CPUCompressor decompressor;
    compressor.co::base::Compressor::initCompressor( nameCompressor );
    decompressor.co::base::Compressor::initDecompressor( nameCompressor );

    const uint64_t flags = EQ_COMPRESSOR_DATA_1D;    
    uint64_t inDims[2]  = { 0, size };
    
    float timeCompress = 0;
    float timeDecompress = 0;
    uint64_t compressedSize = 0;

    co::base::Clock clock;
    compressor.compress( const_cast<uint8_t*>(data), inDims, flags );
    timeCompress = clock.getTimef();

    const unsigned numResults = compressor.getNumResults( );

    std::vector< void * > vectorVoid;
    vectorVoid.resize( numResults );

    std::vector< uint64_t > vectorSize;
    vectorSize.resize(numResults);

    compressedSize = 0;        
    for( unsigned i = 0; i < numResults ; i++ )
    {
        compressor.getResult( i, &vectorVoid[i], &vectorSize[i] );
        compressedSize += vectorSize[i];
    }
        
    co::base::Bufferb result;
    result.resize( size );
    uint8_t* outData = result.getData();
        
    clock.reset();
    decompressor.decompress( &vectorVoid.front(), &vectorSize.front(),
                             numResults, outData, inDims );

    timeDecompress += clock.getTimef();
    compare( outData, data, size );

    _compressedSize += compressedSize;
    _timeCompress += timeCompress;
    _timeDecompress += timeDecompress;   
}

void testCompressorFile()
{
    std::vector< uint32_t >compressorNames = 
        getCompressorNames( EQ_COMPRESSOR_DATATYPE_BYTE );

    std::vector< std::string > files;
    
    getFiles( "", files, "*.dll" );
    getFiles( "", files, "*.exe" );
    getFiles( "", files, "*.so" );
    getFiles( "text", files, "*.a" );
    getFiles( "text", files, "*.dylib" );
    
    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "                File, Compressor,       SIZE, "
              << "Compressed,     t_comp,   t_decomp" << std::endl;
    
    for( std::vector< uint32_t >::const_iterator i = compressorNames.begin();
         i != compressorNames.end(); ++i )
    {
        uint64_t totalResult = 0;
        uint64_t totalSize = 0;
        float totalCompressionTime = 0;
        float totalDecompressionTime = 0;

        for ( std::vector< std::string >::const_iterator j = files.begin();
              j != files.end(); ++j )
        {
            co::base::Bufferb* data = readDataFile( *j );
            const std::string name = co::base::getFilename( *j );
            
            if( data == 0 )
                continue;
            
            uint64_t compressedSize = 0;
            float compressTime = 0.f;
            float decompressTime = 0.f;

            testCompressByte( *i, data->getData(), data->getSize(),
                              compressedSize, compressTime, decompressTime );

            totalSize += data->getSize();
            totalResult += compressedSize;
            totalCompressionTime += compressTime;
            totalDecompressionTime = decompressTime;

            std::cout  << std::setw(20) << name << ", 0x" 
                       << std::setw(8) << std::setfill( '0' )
                       << std::hex << *i << std::dec << std::setfill(' ')
                       << ", " << std::setw(10) << data->getSize() << ", "
                       << std::setw(10) << compressedSize << ", " 
                       << std::setw(10) << compressTime << ", "
                       << std::setw(10) << decompressTime << std::endl;
    
        }
        std::cout << std::setw(24) << "Total: 0x" << std::setw(8)
                  << std::setfill( '0' ) << std::hex << *i << std::dec
                  << std::setfill(' ') << ", " << std::setw(10) << totalSize
                  << ", " << std::setw(10) << totalResult << ", "
                  << std::setw(10) << totalCompressionTime << ", "
                  << std::setw(10) << totalDecompressionTime << std::endl
                  << std::endl;
    }
}


void compare( const uint8_t *dst, const uint8_t *src, const uint32_t nbytes )
{
    for( uint64_t i = 0; i < nbytes; ++i )
    {
        TESTINFO( *dst == *src,
                  int( *dst ) << " != " << int( *src ) << " @ " << i );
        ++dst;
        ++src;
    }
}

std::vector< std::string > getFiles( const std::string path, 
                                     std::vector< std::string >& files, 
                                     const std::string& ext )
{
    const co::base::PluginRegistry& reg = co::base::Global::getPluginRegistry();
    const co::base::Strings& paths = reg.getDirectories();
    for ( uint64_t j = 0; j < paths.size(); j++)
    {
        co::base::Strings candidates = 
            co::base::searchDirectory( paths[j], ext.c_str() );
        for( co::base::Strings::const_iterator i = candidates.begin();
                i != candidates.end(); ++i )
        {
            const std::string& filename = *i;
            files.push_back( paths[j] + '/' + filename );
        }    
    }
    return files;
}

co::base::Bufferb* readDataFile( const std::string& filename )
{
    co::base::MemoryMap file;
    const uint8_t* addr = static_cast< const uint8_t* >( file.map( filename ));

    if( !addr )
    {
        EQERROR << "Can't open " << filename << " for reading" << std::endl;
        return 0;
    }

    const size_t size = file.getSize();    
    co::base::Bufferb* data = new co::base::Bufferb();

    // read the file
    data->resize( size );
    memcpy( data->getData(), addr, size);

    return data;
}
