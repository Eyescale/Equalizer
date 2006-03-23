
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_CONNECTION_H
#define EQS_LOADER_CONNECTION_H

#include "connectionDescription.h"
#include "global.h"

#include <boost/spirit/symbols/symbols.hpp>

namespace eqLoader
{
    struct connectionType : boost::spirit::symbols<int32_t>
    {
        connectionType()
            {
                add
                    ("TCPIP"    , eqNet::Connection::TYPE_TCPIP )
                    ("PIPE"     , eqNet::Connection::TYPE_PIPE )
                    ;
            }
    } connectionType_p;

    struct setGlobalConnectionType
    {
        setGlobalConnectionType() {}

        void operator()( int32_t n ) const
            {
                eqs::Global::instance()->setConnectionIAttribute( 
                    eqs::ConnectionDescription::IATTR_TYPE, n );
            }
    };

    struct setConnectionType
    {
        setConnectionType( State& state ) : _state( state ) {}

        void operator()( int32_t n ) const
            {
                _state.connectionDescription->type = (eqNet::Connection::Type)n;
            }
    
        State& _state;
    };
}

#endif // EQS_LOADER_CONNECTION_H
