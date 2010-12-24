
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "compressor.h"

namespace eq
{
namespace plugin
{
namespace
{
    typedef std::vector< Compressor::Functions > Compressors;
    static Compressors* _functions;

    const Compressor::Functions& _findFunctions( const unsigned name )
    {
        for( Compressors::const_iterator i = _functions->begin();
             i != _functions->end(); ++i )
        {
            const Compressor::Functions& functions = *i;
            if( functions.name == name )
                return functions;
        }

        assert( 0 ); // UNREACHABLE
        return _functions->front();
    }
}

Compressor::Compressor()
{}

Compressor::~Compressor()
{
    for ( size_t i = 0; i < _results.size(); i++ )
        delete ( _results[i] );

    _results.clear();
}

void Compressor::registerEngine( const Compressor::Functions& functions )
{
    co::plugin::Compressor::registerEngine( functions );
}

}
}


