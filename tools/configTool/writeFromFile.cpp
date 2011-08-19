
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "configTool.h"
#include "frame.h"

#include <eq/server/equalizers/framerateEqualizer.h>
#include <eq/server/canvas.h>
#include <eq/server/global.h>
#include <eq/server/layout.h>
#include <eq/server/node.h>
#include <eq/server/segment.h>
#include <eq/server/view.h>
#include <eq/server/window.h>

#include <eq/fabric/viewport.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) ((a)>(b)?(a):(b))
#endif


using namespace std;
using namespace eq::server;


/** 2D decomposition based on precalculated grid */
static void _mode2D(       Config*        config,
                     const vector<float>& xMarks,
                     const vector<float>& yMarks  )
{
    Compound* compound =  config->getCompounds()[0];

    const size_t rows    = yMarks.size() - 1;
    const size_t columns = xMarks.size() - 1;

    size_t i = 0;
    for( size_t y = 0; y < rows; ++y )
    for( size_t x = 0; x < columns; ++x )
    {
        Compound* child = new Compound( compound );
        std::ostringstream channelName;
        channelName << "channel" << i;

        Channel* childChannel = config->find< Channel >( channelName.str( ));
        child->setChannel( childChannel );

        child->setViewport( 
            eq::Viewport( xMarks[x  ],           yMarks[y  ],
                          xMarks[x+1]-xMarks[x], yMarks[y+1]-yMarks[y] ));

        if( i != 0 )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << i;

            child->addOutputFrame(   ::Frame::create( frameName ));
            compound->addInputFrame( ::Frame::create( frameName ));
        }
        i++;
    }
}

/** Pure DB rendering when each node compose result from its pipes and transmit
    only one set of images to the destination node. */
static void _modeDB(       Config*  config,
                     const unsigned nChannels,
                     const unsigned nPipes    )
{
    Compound* compound = config->getCompounds()[0];

    vector<float> ranges( nChannels + 1, 0 );
    ranges[ nChannels ] = 1.0;
    for( unsigned i = 1; i < nChannels; ++i )
        ranges[ i ] = ranges[ i-1 ] + 1.0/nChannels;

    unsigned i = 0;
    // for each node
    for( unsigned n = 0 ; n < nChannels/nPipes; ++n )
    {
        Compound* childNode = compound;

        if( n != 0 ) // don't create separate compound for dst channel
        {
            childNode = new Compound( compound );
            std::ostringstream channelName;
            channelName << "channel" << n*nPipes;

            Channel* childChannel = config->find< Channel >( channelName.str());
            childNode->setChannel( childChannel );
        }

        // for each gpu on the node
        for( unsigned p = 0; p < nPipes; ++p )
        {
            Compound* childPipe = new Compound( childNode );
            childPipe->setRange( eq::Range( ranges[ i ], ranges[ i+1 ] ));

            if( i != n*nPipes )
            {
                std::ostringstream channelName;
                channelName << "channel" << i;

                Channel* childChannel =
                    config->find< Channel >( channelName.str( ));
                childPipe->setChannel( childChannel );

                std::ostringstream frameName;
                frameName << "frame.channel" << i;

                childPipe->addOutputFrame( ::Frame::create( frameName ));
                childNode->addInputFrame(  ::Frame::create( frameName ));
            }
            i++;
        }
        if( n != 0 ) // dst channel has no output
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << n*nPipes;

            childNode->addOutputFrame( ::Frame::create( frameName ));
            compound->addInputFrame(   ::Frame::create( frameName ));
        }
    }
}

/** DB_ds rendering when each node compose result from its pipes on to first 
    pipe, then result is used in DB_ds compositing between nodes */
static void _modeDS(       Config*                config,
                     const unsigned               nChannels,
                     const unsigned               nPipes,
                     const vector< vector<int> >& descr,
                     const vector<float>&         xMarks,
                     const vector<float>&         yMarks  )
{
    Compound* compound = config->getCompounds()[0];

    vector<float> ranges( nChannels + 1, 0 );
    ranges[ nChannels ] = 1.0;
    for( unsigned i = 1; i < nChannels; ++i )
        ranges[ i ] = ranges[ i-1 ] + 1.0/nChannels;

    const unsigned nNodes = nChannels/nPipes;
    if( descr.size() < nNodes )
    {
        cerr << "Description file is incomplete" << std::endl;
        return;
    }

    const int rows      = int( yMarks.size( )) - 1;
    const int columns   = int( xMarks.size( )) - 1;
    const int cells     = rows*columns;

    // check that all specified viewports are within a grid
    for( size_t i = 0; i < nNodes; ++i  )
    {
        const vector< int >& vals = descr[i];
        for( size_t j  = 0; j < vals.size(); ++j )
            if( vals[j] >= cells || vals[j] < 0 )
            {
                cerr << "description of region is invalid: "
                     << vals[j] << " no such cell" << std::endl;
                return;
            }
    }

    // fill all viewports for grid
    vector< eq::Viewport > tmpVP;
    for( int y = 0; y < rows; ++y )
        for( int x = 0; x < columns; ++x )
            tmpVP.push_back( eq::Viewport( xMarks[x],
                                           yMarks[y],
                                           xMarks[x+1]-xMarks[x],
                                           yMarks[y+1]-yMarks[y] ));
    // build per-node viewports
    vector< eq::Viewport > vp( nNodes );
    for( size_t i = 0; i < nNodes; ++i  )
    {
        const vector< int >& vals = descr[i];
        vp[i] = tmpVP[vals[0]];

        for( size_t j  = 1; j < vals.size(); ++j )
            vp[i].unite( tmpVP[vals[j]] );
    }

    unsigned i = 0;
    // for each node
    for( unsigned n = 0 ; n < nNodes; ++n )
    {
        Compound* child = compound;

        if( n != 0 ) // don't create separate compound for dst channel
        {
            child = new Compound( compound );
            std::ostringstream channelName;
            channelName << "channel" << n*nPipes;

            Channel* childChannel = config->find< Channel >( channelName.str());
            child->setChannel( childChannel );
        }

        Compound* childNode = new Compound( child );

        // for each gpu on the node
        for( unsigned p = 0; p < nPipes; ++p )
        {
            Compound* childPipe = new Compound( childNode );
            childPipe->setRange( eq::Range( ranges[ i ], ranges[ i+1 ] ));

            if( i != n*nPipes )
            {
                std::ostringstream channelName;
                channelName << "channel" << i;

                Channel* childChannel =
                    config->find< Channel >( channelName.str( ));
                childPipe->setChannel( childChannel );


                std::ostringstream frameName;
                frameName << "frame.channel" << i;

                childPipe->addOutputFrame( ::Frame::create( frameName ));
                childNode->addInputFrame(  ::Frame::create( frameName ));
            }
            i++;
        }

        for( unsigned k = 0; k < nNodes; ++k )
            if( k != n )
            {
                // output parts to compose on other nodes
                std::ostringstream frameName;
                frameName << "fr" << k*nPipes << ".ch" << n*nPipes;

                childNode->addOutputFrame( ::Frame::create( frameName, vp[k] ));

                // input parts from other nodes to compose on current node
                frameName.str("");
                frameName<< "fr" << n*nPipes << ".ch" << k*nPipes;

                child->addInputFrame( ::Frame::create( frameName ));
            }

        // output color result for final compositing on the first node
        if( n != 0 )
        {
            std::ostringstream frameName;
            frameName << "frame.channel" << n*nPipes;

            child->addOutputFrame(   ::Frame::create( frameName, vp[n], true ));
            compound->addInputFrame( ::Frame::create( frameName ));
        }
    }
}

static bool _split( vector<float>& res, unsigned len, unsigned parts,
                                                      unsigned align )
{
    res.resize( parts+1 );

    res[     0 ] =  .0f;
    res[ parts ] = 1.0f;

    unsigned rest = len;
    for( unsigned i = 0; i < parts-1; ++i )
    {
        const unsigned p = parts - i;
        unsigned width = (rest + align - 1) / p;
        width -= width % align;
        rest  -= width;

        if( rest % align != 0 )
        {
            cerr << "rest % align != 0 " << endl;
            return false;
        }
        if( width % align != 0 )
        {
            cerr << "width % align != 0 " << endl;
            return false;
        }
        res[ i+1 ] = (len - rest) / (float)( len );
    }
    return true;
}

/*

 Description file should look like this:

align 16
> 1 2
> 3 4
> 5 6

 this means spatial areas will be aligned with the value specified 
 every next line correspond to a node; numbers correspond to cells 
 from the grid (-C, -R in the command line to specify grid dimensions).
 Values per node only used for DB_ds configs.

 Note: You can use only first line, in which case areas will be assigned
       automatically based on a grid. One cell of grid per area.

*/
void ConfigTool::_writeFromDescription( Config* config ) const
{
    // read description file
    std::ifstream inStream;

    inStream.open( _descrFile.c_str() );
    if( !inStream.is_open() )
    {
        cerr << "Can't open file with info" << endl;
        return;
    }

    unsigned align = 1;
    vector< vector<int> > descr;

    string str;
    while( inStream >> str )
    {
        size_t pos = str.find( "align" );
        if( pos != string::npos )
        {
            inStream >> str;
            istringstream tmpStream( str );
            if( !(tmpStream >> align ))
            {
                cerr << "align specified incorrectly" << endl;
                return;
            }
            if( _resX % align != 0 ||  _resY % align != 0 )
            {
                cerr << "width or height is not devisible by align" << endl;
                return;
            }
            continue;
        }
        // else

        if( str.find( ">" ) != string::npos )
        {
            descr.push_back( vector<int>( ));
            continue;
        }else
            if( descr.size( ) == 0 )
                descr.push_back( vector<int>( ));

        istringstream tmpStream( str );

        int val;
        while( tmpStream >> val )
        {
            descr.back().push_back( val-1 );
        }
    }

    inStream.close();

    if( descr.size() == 0 )
        for( unsigned i = 0; i < _nChannels/_nPipes; ++i )
            descr.push_back( vector<int>( 1, i ));

    vector<float> xMarks;
    vector<float> yMarks;

    if( !_split( xMarks, _resX, _columns, align ))
        return;

    if( !_split( yMarks, _resY, _rows,    align ))
        return;


    Compound* compound = new Compound( config );
    Channel* channel = config->find< Channel >( "channel0" );
    compound->setChannel( channel );

    eq::Wall wall;
    wall.bottomLeft  = eq::Vector3f( -.32f, -.2f, -.75f );
    wall.bottomRight = eq::Vector3f(  .32f, -.2f, -.75f );
    wall.topLeft     = eq::Vector3f( -.32f,  .2f, -.75f );
    compound->setWall( wall );

    if( _mode != MODE_2D )
        compound->setBuffers( eq::Frame::BUFFER_COLOR |
                              eq::Frame::BUFFER_DEPTH );

    switch( _mode )
    {
        case MODE_2D:
            _mode2D( config, xMarks, yMarks );
            break;

        case MODE_DB:
            _modeDB( config, _nChannels, _nPipes );
            break;

        case MODE_DB_DS:
            _modeDS( config, _nChannels, _nPipes, descr, xMarks, yMarks );
            break;

        default:
            cerr << "Unimplemented mode " << static_cast< unsigned >( _mode )
                 << endl;
            exit( EXIT_FAILURE );
    }


}
