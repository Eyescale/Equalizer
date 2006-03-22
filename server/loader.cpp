
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "loader.h"
#include "loaderCompound.h"
#include "loaderConnection.h"
#include "loaderState.h"

#include "channel.h" 
#include "node.h"    
#include "pipe.h"    
#include "server.h"
#include "window.h"  

#include <boost/spirit/core.hpp>
#include <boost/spirit/iterator/file_iterator.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace eqLoader;
using namespace eqs;
using namespace std;
using namespace boost::spirit;

//---------------------------------------------------------------------------
// actions
//---------------------------------------------------------------------------
struct newServerAction
{
    newServerAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        { _state.server = _state.loader->createServer( ); }
    
    State& _state;
};
struct newConfigAction
{
    newConfigAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        {
            _state.config = _state.loader->createConfig( );
            _state.server->addConfig( _state.config );
        }
    
    State& _state;
};
struct newNodeAction
{
    newNodeAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        {
            _state.node = _state.loader->createNode( );
            _state.config->addNode( _state.node );
        }
    
    State& _state;
};
struct newPipeAction
{
    newPipeAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        {
            _state.pipe = _state.loader->createPipe( );
            _state.node->addPipe( _state.pipe );
        }
    
    State& _state;
};
struct newWindowAction
{
    newWindowAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        {
            _state.window = _state.loader->createWindow( );
            _state.pipe->addWindow( _state.window );
        }
    
    State& _state;
};
struct newChannelAction
{
    newChannelAction( State& state ) : _state( state ) {}

    void operator()(const char& c) const
        {
            _state.channel = _state.loader->createChannel( );
            _state.window->addChannel( _state.channel );
        }
    
    State& _state;
};

//---------------------------------------------------------------------------
// grammar
//---------------------------------------------------------------------------
struct eqsGrammar : public grammar<eqsGrammar>
{
    eqsGrammar( State& state ) : _state( state ) {}

    template <typename ScannerT> struct definition
    {
        definition( eqsGrammar const& self )
        {
            global = "global" 
                >> ch_p('{')
                >> *( str_p("EQ_CONNECTION_TYPE") 
                      >> connectionType_p[setConnectionType(self._state)] )
//                 >> *(EQ_CONNECTION_HOSTNAME          "localhost"
//     EQ_CONNECTION_TCPIP_PORT        4242
//     EQ_CONNECTION_LAUNCH_TIMEOUT    100s
//     EQ_CONNECTION_LAUNCH_COMMAND    "ssh -n %h %c >& %h.log"
                >> ch_p('}');

            server = "server"
                >> ch_p('{')[newServerAction(self._state)]
                >> +(config)
                >> ch_p('}');

            config = "config" 
                >> ch_p('{')[newConfigAction(self._state)]
                >> +(node)
                >> +(compound)
                >> ch_p('}');

            node = "node"
                >> ch_p('{')[newNodeAction(self._state)]
                >> +(pipe)
                >> ch_p('}');

            pipe = "pipe"
                >> ch_p('{')[newPipeAction(self._state)]
                >> +(window)
                >> ch_p('}');

            window = "window"
                >> ch_p('{')[newWindowAction(self._state)]
                >> +(channel)
                >> ch_p('}');

            channel = "channel"
                >> ch_p('{')[newChannelAction(self._state)]
                >> ch_p('}');

            compound = "compound"
                >> ch_p('{')[newCompoundAction(self._state)]
                >> *(compoundParams)
                >> *(compound)
                >> ch_p('}')[leaveCompoundAction(self._state)];

            compoundParams = 
                ( "mode" >> ch_p('[') >> compoundMode >> ch_p(']') ) |
                ( "channel" >> ch_p('"') >> compoundMode >> ch_p('"') );

            compoundMode = 
                str_p("SYNC")[compoundSetModeAction( self._state,
                                                     Compound::MODE_SYNC )] |
                str_p("2D")[compoundSetModeAction( self._state,
                                                   Compound::MODE_2D )];
        }

        rule<ScannerT> global;
        rule<ScannerT> server, config, node, pipe, window, channel, compound;
        rule<ScannerT> compoundParams, compoundMode;
        rule<ScannerT> const& start() const { return server; }
    };

    State& _state;
};

//---------------------------------------------------------------------------
// loader
//---------------------------------------------------------------------------
Server* Loader::loadConfig( const string& filename )
{
    if( filename.empty() )
    {
        EQERROR << "Can't load empty config filename" << endl;
        return NULL;
    }

    // Create a file iterator for this file
    file_iterator<char> first( filename );

    if( !first )
    {
        EQERROR << "Can't open config file " << filename << endl;
        return NULL;
    }

    file_iterator<char> last = first.make_end();
    State               state( this );
    eqsGrammar          g( state );
    string              str;
    
    parse_info< file_iterator<char> > info = parse( first, last, g, space_p );

    if( info.full )
        return state.server;
    
    EQERROR << "Parsing of config file " << filename << " failed" << endl;

    if( state.server )
        delete state.server;

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

