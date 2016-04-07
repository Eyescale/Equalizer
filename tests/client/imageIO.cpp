
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

// Tests the functionality of the image load/save

#include <lunchbox/test.h>

#include <eq/nodeFactory.h>
#include <eq/image.h>
#include <eq/init.h>
#include <lunchbox/file.h>
#include <lunchbox/memoryMap.h>
#include <boost/filesystem.hpp>

int main( int argc, char **argv )
{
    // setup
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::Strings images;
    eq::Strings candidates = lunchbox::searchDirectory( "images", ".*\\.rgb");
    for( eq::Strings::const_iterator i = candidates.begin();
        i != candidates.end(); ++i )
    {
        const std::string& filename = *i;
        const size_t decompPos = filename.find( "out_" );
        if( decompPos == std::string::npos )
            images.push_back( "images/" + filename );
    }
    TEST( !images.empty( ));

    eq::Image image;
    // For each image
    for( eq::Strings::const_iterator i = images.begin(); i != images.end(); ++i)
    {
        const std::string& inFilename = *i;
        const boost::filesystem::path path( inFilename );
        const std::string outFilename = path.parent_path().string() + "/out_" +
#if BOOST_FILESYSTEM_VERSION == 3
                                        path.filename().string();
#else
                                        path.filename();
#endif

        TEST( image.readImage( inFilename, eq::Frame::BUFFER_COLOR ));
        TEST( image.writeImage( outFilename, eq::Frame::BUFFER_COLOR ));

        lunchbox::MemoryMap orig;
        lunchbox::MemoryMap copy;
        const uint8_t* origPtr = reinterpret_cast< const uint8_t* >(
                                     orig.map( inFilename ));
        const uint8_t* copyPtr = reinterpret_cast< const uint8_t* >(
                                     copy.map( outFilename ));

        TESTINFO( origPtr, inFilename );
        TESTINFO( copyPtr, inFilename );
        TESTINFO( orig.getSize() - 512 ==
                  image.getPixelDataSize( eq::Frame::BUFFER_COLOR ),
                  inFilename << " " << orig.getSize() - 512 << " != " <<
                  image.getPixelDataSize( eq::Frame::BUFFER_COLOR ));
        TESTINFO( orig.getSize() == copy.getSize(), inFilename);
        TESTINFO( orig.getSize() > 512, inFilename );
        TESTINFO( memcmp( origPtr+512, copyPtr+512, orig.getSize() - 512 ) == 0,
                  inFilename );
    }

    eq::exit();
    return EXIT_SUCCESS;
}
