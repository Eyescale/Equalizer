
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKETFUNC_H
#define EQNET_PACKETFUNC_H

#include <eq/net/commandResult.h>

namespace eqNet
{
    class  Node;
    struct Packet;

    template< typename T > class PacketFunc
    {
    public:
        PacketFunc( T* object, 
                    CommandResult (T::*func)( Node*, const Packet* ))
            : _object( object ), _func( func ) {}

        template< typename O > PacketFunc( const O& from )
                : _object( from._object ),
                  _func( 
                      static_cast<CommandResult (T::*)( Node*, const Packet* )>(
                          from._func ))
            {}

        CommandResult operator()( Node* node, const Packet* packet )
        {
            return (_object->*_func)( node, packet );
        }

         // Can't make private because copy constructor needs access. Can't
         // declare friend because C++ does not allow generic template friends
         // in template classes.
        // private:
        T* _object;
        CommandResult (T::*_func)( Node*, const Packet* );
    };

     
}

#endif //EQNET_PACKETFUNC_H
