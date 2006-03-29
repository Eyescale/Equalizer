
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_NODE_H
#define EQS_LOADER_NODE_H

#include "loaderState.h"

#include "node.h"
#include "config.h"

namespace eqLoader
{
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

    struct NodeGrammar : public grammar<NodeGrammar>
    {
        NodeGrammar() {}

        template <typename ScannerT> struct definition
        {
            rule<ScannerT> const& start() const 
                { return node; }

            definition( NodeGrammar const& self )
                {
                    node = "node"
                        >> ch_p('{')[newNodeAction(self._state)]
                        >> +(pipe)
                        >> ch_p('}');
                }

            rule<ScannerT> node;
        };
    };
}

#endif // EQS_LOADER_NODE_H
