
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_CONFIG_H
#define EQS_LOADER_CONFIG_H

#include "loader.h"
#include "loaderClosure.h"

#include "server.h"

namespace eqLoader
{
    struct ConfigGrammar : public grammar<ConfigGrammar, 
                                          grammarClosure::context_t>
    {
        void newConfig( State state )
            {
                state.config = state.loader->createConfig( );
                state.server->addConfig( state.config );
            }

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> const& start() const 
                { return config; }

            definition( ConfigGrammar const& self )
                {
                    config = "config"
                        >> ch_p('{')[self.newConfig( self.state )]
                        >> ch_p('}');
                }

            rule<ScannerT> config;
        };
    };

//     inline ConfigGrammar config_g( Loader* loader )
//     { 
//         return ConfigGrammar( loader ); 
//     }
}

#endif // EQS_LOADER_CONFIG_H
