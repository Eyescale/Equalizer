/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <eq/client/viewport.h>

#ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) ((a)>(b)?(a):(b))
#endif


using namespace std;

// levels of indention
static string _s0("        ");
static string _s1("            ");
static string _s2("                ");
static string _s3("                    ");

// endl
static string _e( "\n" );

/** 2D decomposition based on precalculated grid */
static void _mode2D( const vector<float>& xMarks,
                     const vector<float>& yMarks  )
{
    const unsigned raws      = yMarks.size() - 1;
    const unsigned columns   = xMarks.size() - 1;

    unsigned i = 0;
    for( unsigned y = 0; y < raws; ++y )
    for( unsigned x = 0; x < columns; ++x )
    {
        cout <<_s1<< "compound" << _e
             <<_s1<< "{" << _e
             <<_s1<< "    channel   \"channel" << i << "\"" << _e
             <<_s1<< "    viewport  [ " << xMarks[x  ]           << " "
                                        << yMarks[y  ]           << " "
                                        << xMarks[x+1]-xMarks[x] << " "
                                        << yMarks[y+1]-yMarks[y] << " ]" <<_e;
        if( i != 0 )
        {
            cout
             <<_s1<< "    outputframe{ name \"frame.channel"<< i <<"\"}" <<_e;
        }
        cout <<_s1<< "}" << _e;

        if( i != 0 )
        {
            cout
             <<_s1<< "inputframe{ name \"frame.channel" << i <<"\" }" <<_e<<_e;
        }
        i++;
    }
}

/** Pure DB rendering when each node compose result from its pipes and transmit
    only one set of images to the destination node. */
static void _modeDB( const unsigned nChannels,
                     const unsigned nPipes    )
{
    vector<float> ranges( nChannels + 1, 0 );
    ranges[ nChannels ] = 1.0;
    for( unsigned i = 1; i < nChannels; ++i )
        ranges[ i ] = ranges[ i-1 ] + 1.0/nChannels;

    unsigned i = 0;
    // for each node
    for( unsigned n = 0 ; n < nChannels/nPipes; ++n )
    {
        if( n != 0 ) // don't create separate compound for dst channel
        {
            cout
             <<_s1<< "compound" <<_e
             <<_s1<< "{" <<_e
             <<_s1<< "    channel \"channel" << n*nPipes << "\"" <<_e;
        }

        const string s = (n == 0) ? _s1 : _s2;
    // for each gpu on the node
    for( unsigned p = 0; p < nPipes; ++p )
    {
        cout <<s<< "compound" <<_e
             <<s<< "{" <<_e;

        if( i != n*nPipes )
        {
            cout
             <<s<< "    channel \"channel" << i << "\"" <<_e;
        }
        cout <<s<< "    range [ " << ranges[ i   ] << " " 
                                  << ranges[ i+1 ] << " ]" <<_e;

        if( i != n*nPipes ) 
        {
            cout
             <<s<< "    outputframe{ name \"frame.channel" << i <<"\"}" <<_e;
        }
        cout <<s<< "}" <<_e;

        if( i != n*nPipes )
        {
            cout
             <<s<< "inputframe{ name \"frame.channel" << i <<"\" }" <<_e<<_e;
        }

        i++;
    }
        if( n != 0 ) // dst channel has no output
        {
            cout
             <<_s2<< "outputframe{ name \"frame.channel" << n*nPipes <<"\"}"<<_e
             <<_s1<< "}" <<_e
             <<_s1<< "inputframe{ name \"frame.channel" << n*nPipes <<"\"}"<<_e
             <<_e;
        }
    }
}

static eq::Viewport _unite( const eq::Viewport& vp1, const eq::Viewport& vp2 )
{
    eq::Viewport res;
    res.x = MIN( vp1.x, vp2.x );
    res.y = MIN( vp1.y, vp2.y );

    res.w = MAX( vp1.x+vp1.w, vp2.x+vp2.w ) - res.x;
    res.h = MAX( vp1.y+vp1.h, vp2.y+vp2.h ) - res.y;

    return res;
}


/** DB_ds rendering when each node compose result from its pipes on to first 
    pipe, then result is used in DB_ds compositing between nodes */
static void _modeDS( const unsigned               nChannels,
                     const unsigned               nPipes,
                     const vector< vector<int> >& descr,
                     const vector<float>&         xMarks,
                     const vector<float>&         yMarks  )
{
    vector<float> ranges( nChannels + 1, 0 );
    ranges[ nChannels ] = 1.0;
    for( unsigned i = 1; i < nChannels; ++i )
        ranges[ i ] = ranges[ i-1 ] + 1.0/nChannels;

    const unsigned nNodes = nChannels/nPipes;
    if( descr.size() < nNodes )
    {
        cerr << "Description file is incomplete" <<_e;
        return;
    }

    const unsigned raws      = yMarks.size() - 1;
    const unsigned columns   = xMarks.size() - 1;
    const int      cells     = raws*columns;

    // check that all specifeid viewports are within a grid
    for( size_t i = 0; i < nNodes; ++i  )
    {
        const vector< int >& vals = descr[i];
        for( size_t j  = 0; j < vals.size(); ++j )
            if( vals[j] >= cells || vals[j] < 0 )
            {
                cerr << "description of region is invalid: "
                     << vals[j] << " no such cell" << _e;
                return;
            }
    }

    // fill all viewports for grid
    vector< eq::Viewport > tmpVP;
    for( unsigned y = 0; y < raws; ++y )
        for( unsigned x = 0; x < columns; ++x )
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
            vp[i] = _unite( vp[i], tmpVP[vals[j]] );
    }

    unsigned i = 0;
    // for each node
    for( unsigned n = 0 ; n < nNodes; ++n )
    {
        if( n != 0 ) // don't create separate compound for dst channel
        {
            cout
             <<_s1<< "compound" <<_e
             <<_s1<< "{" <<_e
             <<_s1<< "    channel   \"channel" << n*nPipes << "\"" <<_e;
        }

        const string sT = (n == 0) ? _s1 : _s2;
        cout
         <<sT<< "compound" <<_e
         <<sT<< "{" <<_e;

        const string s = (n == 0) ? _s2 : _s3;

    // for each gpu on the node
    for( unsigned p = 0; p < nPipes; ++p )
    {

        cout <<s<< "compound" <<_e
             <<s<< "{" <<_e;

        if( i != n*nPipes )
        {
            cout
             <<s<< "    channel \"channel" << i << "\"" <<_e;
        }
        cout <<s<< "    range [ " << ranges[ i   ] << " " 
                                      << ranges[ i+1 ] << " ]" <<_e;

        if( i != n*nPipes ) // output all except first gpu per node
        {
            cout
             <<s<< "    outputframe{ name \"frame.channel" << i <<"\" }" <<_e;
        }
        cout <<s<< "}" <<_e;

        if( i != n*nPipes ) // compose results from other gpu's on to first
        {
            cout
             <<s<< "inputframe{ name \"frame.channel" << i <<"\" }" <<_e<<_e;
        }

        i++;
    }
        // output parts to compose on othe nodes
        for( unsigned k = 0; k < nNodes; ++k )
            if( k != n )
            {
                const eq::Viewport& v = vp[k];
                cout
                 <<s<< "outputframe{ name \"fr" << k*nPipes << ".ch" << n*nPipes
                    << "\" viewport[ "<<v.x<<" "<<v.y<<" "<<v.w<<" "<<v.h <<" ]"
                    << " }"<<_e;
            }

        cout
         <<sT<< "}" <<_e;

        // input parts from other nodes to compose on current node
        for( unsigned k = 0; k < nNodes; ++k)
            if( k != n )
            {
                cout
                 <<sT<< "inputframe{ name \"fr" << n*nPipes << ".ch" <<k*nPipes
                    << "\" }"<<_e;
            }

        // output color result for final compositing on the first node
        if( n != 0 )
        {
            const eq::Viewport& v = vp[n];
            cout
             <<sT<< "outputframe{ name \"frame.channel" << n*nPipes
                 << "\"" << " buffer [ COLOR ] "
                 << "viewport[ "<<v.x<<" "<<v.y<<" "<<v.w<<" "<<v.h <<" ]"
                 << " }" <<_e
             <<_s1<< "}" <<_e
             <<_s1<< "inputframe{ name \"frame.channel" << n*nPipes <<"\" }"<<_e
             <<_e;
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

*/
void ConfigTool::_writeFromDescription() const
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

    vector<float> xMarks;
    vector<float> yMarks;

    if( !_split( xMarks, _resX, _columns, align ))
        return;

    if( !_split( yMarks, _resY, _rows,    align ))
        return;


    cout <<_s0<< "compound" <<_e
         <<_s0<< "{" <<_e
         <<_s0<< "    channel \"channel0\"" <<_e;

    if( _mode != MODE_2D )
    cout <<_s0<< "    buffer [ COLOR DEPTH ]" <<_e;

    cout <<_s0<< "    wall" <<_e
         <<_s0<< "    {" <<_e
         <<_s0<< "        bottom_left  [ -.32 -.20 -.75 ]" <<_e
         <<_s0<< "        bottom_right [  .32 -.20 -.75 ]" <<_e
         <<_s0<< "        top_left     [ -.32  .20 -.75 ]" <<_e
         <<_s0<< "    }" <<_e;

    switch( _mode )
    {
        case MODE_2D:
            _mode2D( xMarks, yMarks );
            break;

        case MODE_DB:
            _modeDB( _nChannels, _nPipes );
            break;

        case MODE_DB_DS:
            _modeDS( _nChannels, _nPipes, descr, xMarks, yMarks );
            break;

        default:
            cerr << "Unimplemented mode " << static_cast< unsigned >( _mode )
                 << endl;
            exit( EXIT_FAILURE );
    }

    cout <<_s0<< "}" <<_e;
}
