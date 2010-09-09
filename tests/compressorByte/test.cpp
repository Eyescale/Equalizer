
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

#include <eq/plugins/compressor.h>

#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>

#include <eq/base/buffer.h>
#include <eq/base/clock.h>
#include <eq/base/file.h>
#include <eq/base/global.h>
#include <eq/base/pluginRegistry.h>
#include <eq/base/types.h>

#include <iostream>  // for std::cerr
#include <numeric>
#include <fstream>

#include "base/compressorInfo.h" // private header
#include "base/plugin.h" // private header

void testCompressByte( const uint32_t nameCompressor,
                       char* data, uint64_t size,
                       std::ofstream* logFile );

void testCompressorFile(  std::ofstream* logFile );
std::vector< uint32_t > getCompressorNames( const uint32_t tokenType );
bool compare( const char *dst, const char *src, const uint32_t nbytes );

std::vector< std::string > getFiles( const std::string path, 
                                     std::vector< std::string >& files, 
                                     const std::string& ext );

eq::base::Bufferb* readDataFile( const std::string& filename );

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    eq::init( argc, argv, &nodeFactory );
    
    std::ofstream* logFile = new std::ofstream( "result.html" );
    *logFile << "<html><head><title>Result of Compression TEST</title>" 
             << "</head><center><H1>Result of Compression TEST<H1></center>"
             << "<body>";
    
    testCompressorFile( logFile );

    *logFile << "</TABLE>";
    *logFile << "</body></html>";
    logFile->close();
    
    eq::exit();
    return EXIT_SUCCESS;
}

std::vector< uint32_t > getCompressorNames( const uint32_t tokenType )
{
    const eq::base::PluginRegistry& registry = eq::base::Global::getPluginRegistry();
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
                       char* data, uint64_t size,
                       std::ofstream* logFile )
{
    eq::base::Clock clock;
    eq::base::Bufferb destfile;

    // find compressor in the corresponding plugin
    eq::base::PluginRegistry& registry = eq::base::Global::getPluginRegistry();
    eq::base::Plugin* plugin = registry.findPlugin( nameCompressor );
    void* instanceComp = plugin->newCompressor( nameCompressor );
    void* instanceUncomp = plugin->newDecompressor( nameCompressor );
    
    const uint64_t flags = EQ_COMPRESSOR_DATA_1D;
    const char* dataorigine = reinterpret_cast<const char*>( data );
    uint64_t inDims[2]  = { 0, size }; 
    clock.reset();
    plugin->compress( instanceComp, nameCompressor, 
                          data, inDims, flags );

    uint64_t time = clock.getTime64();
    const size_t numResults = plugin->getNumResults( 
                                                instanceComp, nameCompressor );

    uint64_t totalSize = 0;

    std::vector< void * > vectorVoid;
    vectorVoid.resize(numResults);
    std::vector< uint64_t > vectorSize;
    vectorSize.resize(numResults);
    for( size_t i = 0; i < numResults ; i++ )
    {
        plugin->getResult( instanceComp, nameCompressor,
                               i, 
                               &vectorVoid[i], 
                               &vectorSize[i] );
        totalSize += vectorSize[i];
    }
    
    std::cout   << totalSize << ", " << std::setw(10)
                << time << ", " << std::setw(10);

    *logFile << "<TD> " << totalSize <<"</TD>";
    *logFile << "<TD> " << time <<"</TD>";
    eq::base::Bufferb result;
    result.resize( size );
    void* outData = reinterpret_cast< uint8_t* >( result.getData() );
    clock.reset();
    
    plugin->decompress( instanceUncomp,
                            nameCompressor,
                            &vectorVoid.front(),
                            &totalSize,
                            numResults, 
                            outData, 
                            inDims, 
                            flags );
    
    time = clock.getTime64();
    std::cout  << time << std::endl;
    *logFile << "<TD> " << time <<"</TD>";
    char* outData8 = reinterpret_cast< char* >( outData );

    if ( compare( outData8, dataorigine, size ) )
        *logFile << "<TD> " << "OK" <<"</TD>";
    else
        *logFile << "<TD> " << "KO" <<"</TD>";
        

}

void testCompressorFile(  std::ofstream* logFile )
{
    std::vector< uint32_t >compressorNames = 
        getCompressorNames( EQ_COMPRESSOR_DATATYPE_BYTE );

    std::vector< std::string > files;
    
    getFiles( "", files, "*.dll" );
    getFiles( "text", files, "*.txt" );
    getFiles( "", files, "*.exe" );
    getFiles( "", files, "*.so" );
    getFiles( "text", files, "*.a" );
    getFiles( "text", files, "*.dylib" );
    
 
    *logFile << "<TABLE BORDER=""1"">";
    *logFile << "<CAPTION> Statistic for compressor bytes </CAPTION>";
    *logFile << "<TR>";
    *logFile << "<TH> File </TH>";
    *logFile << "<TH> Name </TH>";
    *logFile << "<TH> Size </TH>";
    *logFile << "<TH> Compressed size </TH>";
    *logFile << "<TH> T compress [mS]</TH>";
    *logFile << "<TH> T uncompress [mS]</TH>";
    *logFile << "<TH> State</TH>";
    *logFile << "</TR>";
    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "File,              IMAGE,    SIZE, "
              << " COMPRESSED,     t_comp,   t_decomp" << std::endl;
    for ( std::vector< std::string >::const_iterator i = files.begin();
          i != files.end(); i++ )
    {
        eq::base::Bufferb* datas = readDataFile( *i );
        for ( std::vector< uint32_t >::const_iterator j = compressorNames.begin();
          j != compressorNames.end(); j++ )
        {
            *logFile << "<TR>";
            *logFile << "<TH> " << *i  <<"</TH>";
            *logFile << "<TD> " << *j  <<"</TD>";
            *logFile << "<TD> " << datas->getSize() <<"</TD>";

            std::cout  << std::setw(2) << *i << ", " << std::setw( 5 )
                       << *j  << ", " << std::setw(5) 
                       << datas->getSize() << ", ";
            testCompressByte( *j, reinterpret_cast<char*>(datas->getData()), 
                              datas->getSize(), logFile );
            *logFile << "</TR>";
            
        }
    }
    *logFile << "</TABLE>";
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
        eq::base::Strings candidats = eq::base::searchDirectory( paths[j], ext.c_str() );
        for( eq::base::Strings::const_iterator i = candidats.begin();
                i != candidats.end(); ++i )
        {
            const std::string& filename = *i;
            files.push_back( paths[j] + "/" + filename );
        }    
    }
    return files;
}

eq::base::Bufferb* readDataFile( const std::string& filename )
{
    eq::base::Bufferb* datas = new eq::base::Bufferb();
    std::ifstream file( filename.c_str(),  std::ios::in | std::ios::binary );

    // get size
    TESTINFO( file.seekg( 0, std::ios::end ).good(),
                                "Problems with reading file " << filename );
    size_t size = file.tellg();
    TESTINFO( file.seekg( 0, std::ios::beg ).good(),
                                "Problems with reading file " << filename );
    size -= file.tellg();

    // read the file
    datas->resize( size );
    file.read( (char*)(datas->getData()), size );
    file.close();

    return datas;
}
