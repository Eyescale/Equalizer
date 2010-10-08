
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>

#include <eq/base/buffer.h>
#include <eq/base/clock.h>
#include <eq/base/file.h>
#include <eq/base/global.h>
#include <eq/base/memoryMap.h>
#include <eq/base/pluginRegistry.h>
#include <eq/base/types.h>

#include <iostream>  // for std::cerr
#include <numeric>
#include <fstream>
#include <sstream>
#include <string>

#include "base/compressorInfo.h" // private header
#include "base/cpuCompressor.h" // private header
#include "base/plugin.h" // private header

void testCompressByte( const uint32_t nameCompressor,
                       char* data, const uint64_t size,
                       uint64_t& compressedSize,
                       float& timeCompress, 
                       float& timeDecompress );

void testCompressorFile( );
std::vector< uint32_t > getCompressorNames( const uint32_t tokenType );
bool compare( const char *dst, const char *src, const uint32_t nbytes );

std::vector< std::string > getFiles( const std::string path, 
                                     std::vector< std::string >& files, 
                                     const std::string& ext );

eq::base::Bufferb* readDataFile( const std::string& filename );

static size_t numTests = 1;

int main( int argc, char **argv )
{
    if( argc >= 3 && strcmp( argv[1], "-t" ) == 0 )
        numTests = atoi( argv[2] );

    eq::NodeFactory nodeFactory;
    eq::init( argc, argv, &nodeFactory );
    testCompressorFile( );
    eq::exit();

    return EXIT_SUCCESS;
}

std::vector< uint32_t > getCompressorNames( const uint32_t tokenType )
{
    eq::base::PluginRegistry& registry = eq::base::Global::getPluginRegistry();
    const eq::base::Plugins& plugins = registry.getPlugins();

    std::vector< uint32_t > names;
    for( eq::base::Plugins::const_iterator i = plugins.begin();
         i != plugins.end(); ++i )
    {
        const eq::base::CompressorInfos& infos = (*i)->getInfos();
        for( eq::base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            if ( (*j).tokenType == tokenType )
                names.push_back( (*j).name );
        }
    }
    
    return names;
}

void testCompressByte( const uint32_t nameCompressor,
                       char* data, const uint64_t size,
                       uint64_t& _compressedSize,
                       float& _timeCompress, 
                       float& _timeDecompress )
{
    eq::base::CPUCompressor compressor;
    eq::base::CPUCompressor decompressor;
    compressor.initCompressor( nameCompressor );
    decompressor.initDecompressor( nameCompressor );

    const uint64_t flags = EQ_COMPRESSOR_DATA_1D;
    const char* dataorigine = reinterpret_cast<const char*>( data );
    
    uint64_t inDims[2]  = { 0, size };
    
    float timeCompress = 0;
    float timeDecompress = 0;
    bool compareResult = true;
    eq::base::Clock clock;
    uint64_t compressedSize = 0;
    for( size_t j = 0; j < numTests ; j++ )
    {
        clock.reset();
        compressor.compress( data, inDims, flags );

        timeCompress += clock.getTimef();
        const size_t numResults = compressor.getNumResults( );

        uint64_t totalSize = 0;
        std::vector< void * > vectorVoid;

        vectorVoid.resize(numResults);
        std::vector< uint64_t > vectorSize;
        vectorSize.resize(numResults);
        
        for( size_t i = 0; i < numResults ; i++ )
        {
            compressor.getResult( i, &vectorVoid[i], &vectorSize[i] );
            totalSize += vectorSize[i];
        }
        
        compressedSize += totalSize;
        eq::base::Bufferb result;
        result.resize( size );
        void* outData = reinterpret_cast< uint8_t* >( result.getData() );
        clock.reset();
        
        decompressor.decompress( &vectorVoid.front(), &totalSize,
                                 numResults, outData, inDims, flags );
        
        timeDecompress += clock.getTimef();
        
        char* outData8 = reinterpret_cast< char* >( outData );
        compareResult = compare( outData8, dataorigine, size );
    }

    std::cout << std::setw(10) << compressedSize / numTests << ", " 
              << std::setw(7) << timeCompress / numTests << ", "
              << std::setw(7) << timeDecompress / numTests << std::endl;
    
    _compressedSize += compressedSize / numTests;
    _timeCompress += timeCompress / numTests;
    _timeDecompress += timeDecompress / numTests;   
    
    TEST( compareResult );
}

void testCompressorFile( )
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
    std::cout << "               File,          Compressor,       SIZE, "
              << "    Compressd, t_comp,    t_decomp" << std::endl;
    
    for ( std::vector< uint32_t >::const_iterator j = 
               compressorNames.begin(); j != compressorNames.end(); j++ )
    {
        
        uint64_t compressedSize = 0;
        float timeCompress = 0; 
        float timeDecompress = 0;
        uint64_t uncompressedSize = 0;
        for ( std::vector< std::string >::const_iterator i = files.begin();
              i != files.end(); i++ )
        {
            eq::base::Bufferb* datas = readDataFile( *i );
            const std::string name = eq::base::getFilename(*i);
            
            if( datas == 0 )
                continue;
            
            uncompressedSize += datas->getSize() * numTests;
            std::cout  << std::setw(30) << name << ", 0x" 
                       << std::setw(8) << std::setfill( '0' )
                       << std::hex << *j << std::dec << std::setfill(' ')
                       << ", " << std::setw(10) 
                       << datas->getSize() * numTests  << ", ";
            
            testCompressByte( *j, reinterpret_cast<char*>(datas->getData()), 
                              datas->getSize(), compressedSize, timeCompress, 
                              timeDecompress );
            
        }
        std::cout  << std::setw(30) << "Total : "<< ", 0x" 
                   << std::setw(8) << std::setfill( '0' )
                   << std::hex << *j << std::dec << std::setfill(' ')
                   << ", " << std::setw(10) 
                   << uncompressedSize  << ", ";

        std::cout << std::setw(10) << compressedSize << ", " 
                  << std::setw(7) << timeCompress << ", " << std::setw(7);
        std::cout << timeDecompress << std::endl << std::endl;
    }
}


bool compare( const char *dst, const char *src, const uint32_t nbytes )
{
    for ( uint64_t i = 0; i < nbytes; i++ )
    {
        if (*dst != *src)
        {
            std::cerr << "error data in: " << i <<std::endl;
            return false;
        }
        dst++;
        src++;
    }
    return true;
}

std::vector< std::string > getFiles( const std::string path, 
                                     std::vector< std::string >& files, 
                                     const std::string& ext )
{
    const eq::base::Strings& paths = eq::base::Global::getPluginDirectories();
    for ( uint64_t j = 0; j < paths.size(); j++)
    {
        eq::base::Strings candidats = 
            eq::base::searchDirectory( paths[j], ext.c_str() );
        for( eq::base::Strings::const_iterator i = candidats.begin();
                i != candidats.end(); ++i )
        {
            const std::string& filename = *i;
            files.push_back( paths[j] + '/' + filename );
        }    
    }
    return files;
}

eq::base::Bufferb* readDataFile( const std::string& filename )
{
    eq::base::MemoryMap file;
    const uint8_t* addr = static_cast< const uint8_t* >( file.map( filename ));

    if( !addr )
    {
        EQERROR << "Can't open " << filename << " for reading" << std::endl;
        return 0;
    }

    const size_t size = file.getSize();    
    eq::base::Bufferb* datas = new eq::base::Bufferb();

    // read the file
    datas->resize( size );
    memcpy( datas->getData(), addr, size);

    return datas;
}
