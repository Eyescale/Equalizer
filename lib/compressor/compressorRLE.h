
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_PLUGIN_COMPRESSORRLE
#define EQ_PLUGIN_COMPRESSORRLE

#include "compressor.h"
//const uint64_t _rleMarker = 0xF3C553FF64F6477Full; // just a random number

#define WRITE_OUTPUT( name )                                            \
    {                                                                   \
        if( name ## Last == _rleMarker )                                \
        {                                                               \
            *(name ## Out) = _rleMarker; ++(name ## Out);               \
            *(name ## Out) = _rleMarker; ++(name ## Out);               \
            *(name ## Out) = name ## Same; ++(name ## Out);             \
        }                                                               \
        else                                                            \
            switch( name ## Same )                                      \
            {                                                           \
                case 0:                                                 \
                    break;                                              \
                case 3:                                                 \
                    *(name ## Out) = name ## Last;                      \
                    ++(name ## Out);                                    \
                    /* fall through */                                  \
                case 2:                                                 \
                    *(name ## Out) = name ## Last;                      \
                    ++(name ## Out);                                    \
                    /* fall through */                                  \
                case 1:                                                 \
                    *(name ## Out) = name ## Last;                      \
                    ++(name ## Out);                                    \
                    break;                                              \
                                                                        \
                default:                                                \
                    *(name ## Out) = _rleMarker;   ++(name ## Out);     \
                    *(name ## Out) = name ## Last; ++(name ## Out);     \
                    *(name ## Out) = name ## Same; ++(name ## Out);     \
                    break;                                              \
            }                                                           \
    }

#define WRITE( name )                                                   \
    if( name == name ## Last && name ## Same != 255 )                   \
        ++(name ## Same );                                              \
    else                                                                \
    {                                                                   \
        WRITE_OUTPUT( name );                                           \
        name ## Last = name;                                            \
        name ## Same = 1;                                               \
    } 

namespace eq
{
namespace plugin
{

    /**
    * An interace for compressor / uncompressor RLE data 
    *
    */
    class CompressorRLE : public Compressor
        {
        public:
            /** @name CompressorRLE */
            /*@{*/
            /** 
             * Compress data with an algorithm RLE.
             */
            CompressorRLE() {}
            
        protected:
            /** Allocate the output arrays conservatively. */
            void _setupResults( const uint32_t nChannels, 
                                const uint64_t inSize );
        };
    
}
}
#endif // EQ_PLUGIN_COMPRESSORRLE
