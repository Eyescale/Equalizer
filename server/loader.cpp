
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

//#define BOOST_SPIRIT_DEBUG

#include "loader.h"
#include "loaderFile.h"

#include "loaderChannel.h"
#include "loaderCompound.h"
#include "loaderConnection.h"
#include "loaderGlobal.h"

#include "channel.h" 
#include "node.h"    
#include "pipe.h"    
#include "server.h"
#include "window.h"  

#include <fstream>
#include <iostream>
#include <string>

using namespace eqs;
using namespace std;
using namespace eqLoader;
using namespace phoenix;

namespace eqLoader
{
    struct skipGrammar : grammar<skipGrammar>
    {
        template <typename ScannerT>
        struct definition
        {
            definition( skipGrammar const& /*self*/)
                {
                    skip
                        =   space_p
                        |   '#' >> *(anychar_p - '\n') >> '\n'
                        |   "//" >> *(anychar_p - '\n') >> '\n'
                        |   "/*" >> *(anychar_p - "*/") >> "*/"
                        ;
                }
            
            rule<ScannerT> skip;
            rule<ScannerT> const& start() const { return skip; }
        };
    };
}

//---------------------------------------------------------------------------
// loader
//---------------------------------------------------------------------------
Server* Loader::loadConfig( const string& filename )
{
#if 0
    if( filename.empty() )
    {
        EQERROR << "Can't load empty config filename" << endl;
        return NULL;
    }

    // Create a file iterator for this file
    iterator_t first( filename );

    if( !first )
    {
        EQERROR << "Can't open config file " << filename << endl;
        return NULL;
    }

    iterator_t last = first.make_end();
    State           state( this );
    fileGrammar     grammar;
    skipGrammar     skipper;
    
    parse_info< iterator_t > info = parse( first, last, 
                                           grammar,//[var(state) = arg1], 
                                           skipper );

    if( info.full )
        return state.server;
    
    EQERROR << "Parsing of config file " << filename << " failed" << endl;

    if( state.server )
        delete state.server;
#endif
    return NULL;
}



//---------------------------------------------------------------------------
// factory methods
//---------------------------------------------------------------------------
Server*   Loader::createServer()
{
    return new Server;
}

Config*   Loader::createConfig()
{
    return new Config;
}

Node*     Loader::createNode()
{
    return new Node;
}

Pipe*     Loader::createPipe()
{
    return new Pipe;
}

Window*   Loader::createWindow()
{
    return new Window;
}

Channel*  Loader::createChannel()
{
    return new Channel;
}

Compound* Loader::createCompound()
{
    return new Compound;
}

