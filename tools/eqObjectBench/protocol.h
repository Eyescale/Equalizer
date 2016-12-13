
/* Copyright (c) 2016-2017, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <co/nodeCommand.h> // used inline
#include <co/types.h>
#include <pression/data/CompressorInfo.h> // static variable
#include <lunchbox/monitor.h> // member variable
#include <iostream>

namespace plydist
{
enum Commands //!< Commands between Server and Client
{
    CMD_NODE_DONE = co::CMD_NODE_CUSTOM,
    CMD_CLIENT_MAP,
    CMD_CLIENT_SYNC
};

enum class Options : unsigned //!< Object distribution options to benchmark
{
    none = 0,
    instanceCache = 0x1,
    sendOnRegister = 0x2,
    compression = 0x4,
    buffered = 0x8,
    chunked = 0x10,
    multicast = 0x20,
    all = 0x40
};

inline bool operator& ( const Options l, const Options r )
{
    return ( static_cast< unsigned >( l ) & static_cast< unsigned >( r ));
}

inline Options operator| ( const Options l, const Options r )
{
    return static_cast< Options >( static_cast< unsigned >( l ) |
                                   static_cast< unsigned >( r ));
}

inline bool operator& ( const Options l, const size_t r )
{
    return ( static_cast< size_t >( l ) & r );
}

inline Options& operator++ ( Options& options )
{
    if( options == Options::all )
        LBTHROW( std::runtime_error( "Options overflow" ));

    options = static_cast< Options >( static_cast< unsigned >( options ) + 1 );
    return options;
}

inline Options operator<< ( const Options& options, const size_t shift )
{
    if( options >= Options::all )
        LBTHROW( std::runtime_error( "Options overflow" ));

    return static_cast< Options >(
               static_cast< unsigned >( options ) << shift );
}

inline std::ostream& operator << ( std::ostream& os, const Options options )
{
    if( options == Options::none )
        return os << "none ";
    if( options & Options::instanceCache )
        os << "instanceCache ";
    if( options & Options::sendOnRegister )
        os << "sendOnRegister ";
    if( options & Options::compression )
        os << "compression ";
    if( options & Options::buffered )
        os << "buffered ";
    if( options & Options::chunked )
        os << "chunked ";
    if( options & Options::multicast )
        os << "multicast ";
    return os;
}

}
