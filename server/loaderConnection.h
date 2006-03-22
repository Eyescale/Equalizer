
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_CONNECTION_H
#define EQS_LOADER_CONNECTION_H

namespace eqLoader
{
    struct connectionType : symbols<unsigned>
    {
        connectionType()
            {
                add
                    ("TCPIP"    , eqNet::ConnectionDescription::TYPE_TCPIP )
                    ("PIPE"     , eqNet::ConnectionDescription::TYPE_PIPE )
                    ;
            }
    } connectionType_p;

    struct setConnectionType
    {
        setConnectionType( State& state ) : _state( state ) {}

        void operator()( unsigned n ) const
            {
                _state.connectionDescription->type = n;
            }
    
        State& _state;
    };
};

}

#endif // EQS_LOADER_CONNECTION_H
