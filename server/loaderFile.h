
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_FILE_H
#define EQS_LOADER_FILE_H

#include "loaderConfig.h"
#include "loaderClosure.h"

// to be removed
#include "config.h"
#include "node.h"
#include "pipe.h"

namespace eqLoader
{
    //-----------------------------------------------------------------------
    // actions
    //-----------------------------------------------------------------------
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

    //-----------------------------------------------------------------------
    // grammar
    //-----------------------------------------------------------------------
    struct fileGrammar : public grammar<fileGrammar, grammarClosure::context_t>
    {
        template <typename ScannerT> struct definition
        {
            rule<ScannerT> const& start() const { return file; }

            definition( fileGrammar const& self )
                {
                    file = /**(global_g) >> */server;

                    server = "server"
                        >> ch_p('{')//[(self.state).server = 
                                    // (self.state).loader->createServer()]
                        >> +(config(self.state))
                        >> ch_p('}');

//             config = "config" 
//                 >> ch_p('{')[config.config = self._loader->createConfig()]
//                 >> +(node)
//                 >> +(compound)
//                 >> ch_p('}');

//             node = "node"
//                 >> ch_p('{')[newNodeAction(self._state)]
//                 >> +(pipe)
//                 >> ch_p('}');

//             pipe = "pipe"
//                 >> ch_p('{')[newPipeAction(self._state)]
//                 >> +(window)
//                 >> ch_p('}');

//             window = "window"
//                 >> ch_p('{')[newWindowAction(self._state)]
//                 >> (channel_p(self._state))
//                 >> ch_p('}');

//             compound = "compound"
//                 >> ch_p('{')[newCompoundAction(self._state)]
//                 >> *(compoundParams)
//                 >> *(compound)
//                 >> ch_p('}')[leaveCompoundAction(self._state)];

//             compoundParams = 
//                 ( "mode" >> ch_p('[') >> compoundMode >> ch_p(']') ) |
//                 ( "channel" >> ch_p('"') >> compoundMode >> ch_p('"') );

//             compoundMode = 
//                 str_p("SYNC")[compoundSetModeAction( self._state,
//                                                      Compound::MODE_SYNC )] |
//                 str_p("2D")[compoundSetModeAction( self._state,
//                                                    Compound::MODE_2D )];
                }
            
            rule<ScannerT> top;
            rule<ScannerT, grammarClosure::context_t> file;
            rule<ScannerT, grammarClosure::context_t> server;
            ConfigGrammar config;
//         rule<ScannerT> node, pipe, window, channel, compound;
//         rule<ScannerT> compoundParams, compoundMode;
        };
    };
}

#endif // EQS_LOADER_FILE_H
