
/*
 * Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved.   
 *   Code adapted for Equalizer usage, mainly to use
 *   std::vector and other C++-isms
 */

/*************************************************************************
* Name:        lz.c
* Author:      Marcus Geelnard
* Description: LZ77 coder/decoder implementation.
* Reentrant:   Yes
*
* The LZ77 compression scheme is a substitutional compression scheme
* proposed by Abraham Lempel and Jakob Ziv in 1977. It is very simple in
* its design, and uses no fancy bit level compression.
*
* This is my first attempt at an implementation of a LZ77 code/decoder.
*
* The principle of the LZ77 compression algorithm is to store repeated
* occurrences of strings as references to previous occurrences of the same
* string. The point is that the reference consumes less space than the
* string itself, provided that the string is long enough (in this
* implementation, the string has to be at least 4 bytes long, since the
* minimum coded reference is 3 bytes long). Also note that the term
* "string" refers to any kind of byte sequence (it does not have to be
* an ASCII string, for instance).
*
* The coder uses a brute force approach to finding string matches in the
* history buffer (or "sliding window", if you wish), which is very, very
* slow. I recon the complexity is somewhere between O(n^2) and O(n^3),
* depending on the input data.
*
* There is also a faster implementation that uses a large working buffer
* in which a "jump table" is stored, which is used to quickly find
* possible string matches (see the source code for LZ_CompressFast() for
* more information). The faster method is an order of magnitude faster,
* but still quite slow compared to other compression methods.
*
* The upside is that decompression is very fast, and the compression ratio
* is often very good.
*
* The reference to a string is coded as a (length,offset) pair, where the
* length indicates the length of the string, and the offset gives the
* offset from the current data position. To distinguish between string
* references and literal strings (uncompressed bytes), a string reference
* is preceded by a marker byte, which is chosen as the least common byte
* symbol in the input data stream (this marker byte is stored in the
* output stream as the first byte).
*
* Occurrences of the marker byte in the stream are encoded as the marker
* byte followed by a zero byte, which means that occurrences of the marker
* byte have to be coded with two bytes.
*
* The lengths and offsets are coded in a variable length fashion, allowing
* values of any magnitude (up to 4294967295 in this implementation).
*
* With this compression scheme, the worst case compression result is
* (257/256)*insize + 1.
*
*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/



#include "compressor.h"

#include "base.h"
#include "log.h"

using namespace eqBase;
using namespace std;

/*************************************************************************
 * Constants used for LZ77 coding
 *************************************************************************/

/* Maximum offset (can be any size < 2^31). Lower values give faster
   compression, while higher values gives better compression. The default
   value of 100000 is quite high. Experiment to see what works best for
   you. */
#define LZ_MAX_OFFSET 100000

// Internal Function Prototypes
static inline unsigned _LZ_StringCompare( const uint8_t* str1,
                                          const uint8_t* str2, 
                                          const size_t minlen, 
                                          const size_t maxlen );
static inline void _LZ_WriteVarSize( const size_t x, vector<uint8_t>& buffer );
static inline unsigned _LZ_ReadVarSize( const uint8_t* buffer, 
                                        unsigned* position );



void Compressor::compressLZ( const vector<uint8_t>& input,
                             vector<uint8_t>& output )
{
    if( input.empty( ))
        return;

    EQASSERT( input.size() < (100 * 1024 * 1024 )); // < 100MB

    const unsigned insize = input.size();
    unsigned* work = new unsigned[ input.size() + 65536 ];

    /* Assign arrays to the working area */
    unsigned* lastindex = work;
    unsigned* jumptable = &work[ 65536 ];

    /* Build a "jump table". Here is how the jump table works:
       jumptable[i] points to the nearest previous occurrence of the same
       symbol pair as in[i]:in[i+1], so in[i] == in[jumptable[i]] and
       in[i+1] == in[jumptable[i]+1], and so on... Following the jump table
       gives a dramatic boost for the string search'n'match loop compared
       to doing a brute force search. The jump table is built in O(n) time,
       so it is a cheap operation in terms of time, but it is expensice in
       terms of memory consumption. */
    for( unsigned i = 0; i < 65536; ++i )
        lastindex[ i ] = 0xffffffff;

    for( unsigned i = 0; i < insize-1; ++i )
    {
        const unsigned symbols = (static_cast<unsigned>(input[i]) << 8) |
                                 (static_cast<unsigned>(input[i+1]));
        const unsigned index   = lastindex[ symbols ];
        lastindex[ symbols ] = i;
        jumptable[ i ] = index;
    }
    jumptable[ insize-1 ] = 0xffffffff;

    /* Create histogram */
    unsigned histogram[ 256 ] = {0};
    for( unsigned i = 0; i < insize; ++ i )
        ++histogram[ input[ i ] ];

    /* Find the least common byte, and use it as the marker symbol */
    uint8_t marker = 0;
    for( unsigned i = 1; i < 256; ++ i )
        if( histogram[ i ] < histogram[ marker ] )
            marker = i;

    /* reserve size */
    const unsigned start = output.size();
    output.push_back( 0 );
    output.push_back( 0 );
    output.push_back( 0 );
    output.push_back( 0 );
    
    /* Remember the marker symbol for the decoder */
    output.push_back( marker );

    /* Start of compression */
    unsigned inpos = 0;

    /* Main compression loop */
    unsigned bytesleft = insize;
    do
    {
        /* Get pointer to current position */
        const uint8_t* ptr1 = &input[ inpos ];

        /* Search history window for maximum length string match */
        unsigned bestlength = 3;
        unsigned bestoffset = 0;
        unsigned index      = jumptable[ inpos ];
        while( (index != 0xffffffff) && ((inpos - index) < LZ_MAX_OFFSET) )
        {
            /* Get pointer to candidate string */
            const uint8_t* ptr2 = &input[ index ];

            /* Quickly determine if this is a candidate (for speed) */
            if( ptr2[ bestlength ] == ptr1[ bestlength ] )
            {
                /* Determine maximum length for this offset */
                const unsigned offset    = inpos - index;
                const unsigned maxlength = ( bytesleft < offset ? 
                                             bytesleft : offset );

                /* Count maximum length match at this offset */
                const unsigned length    = _LZ_StringCompare( ptr1, ptr2, 2,
                                                              maxlength );

                /* Better match than any previous match? */
                if( length > bestlength )
                {
                    bestlength = length;
                    bestoffset = offset;
                }
            }

            /* Get next possible index from jump table */
            index = jumptable[ index ];
        }

        /* Was there a good enough match? */
        if( (bestlength >= 8) ||
            ((bestlength == 4) && (bestoffset <= 0x0000007f)) ||
            ((bestlength == 5) && (bestoffset <= 0x00003fff)) ||
            ((bestlength == 6) && (bestoffset <= 0x001fffff)) ||
            ((bestlength == 7) && (bestoffset <= 0x0fffffff)) )
        {
            output.push_back( marker );
            _LZ_WriteVarSize( bestlength, output );
            _LZ_WriteVarSize( bestoffset, output );
            inpos     += bestlength;
            bytesleft -= bestlength;
        }
        else
        {
            /* Output single byte (or two bytes if marker byte) */
            const uint8_t symbol = input[ inpos++ ];
            output.push_back( symbol );
            if( symbol == marker )
                output.push_back( 0 );
            -- bytesleft;
        }
    }
    while( bytesleft > 3 );

    /* Dump remaining bytes, if any */
    while( inpos < insize )
    {
        if( input[ inpos ] == marker )
        {
            output.push_back( marker );
            output.push_back( 0 );
        }
        else
        {
            output.push_back( input[ inpos ] );
        }
        ++inpos;
    }

    delete [] work;
    uint32_t* size = reinterpret_cast<uint32_t*>( &output[start] );
    *size = input.size();
}

uint32_t Compressor::decompressLZ( const uint8_t* input,
                                    vector<uint8_t>& output )
{
    const uint32_t size = *reinterpret_cast<const uint32_t*>( input );

    EQASSERT( size > 0 );
    EQASSERT( size < (100 * 1024 * 1024 )); // < 100MB

    const size_t totalSize = output.size() + size;
    output.reserve( totalSize );

    /* Get marker symbol from input stream */
    const uint8_t  marker = input[ 4 ];
    unsigned       inpos  = 5;

    /* Main decompression loop */
    while( output.size() < totalSize )
    {
        const uint8_t symbol = input[ inpos++ ];
        if( symbol == marker )
        {
            /* We had a marker byte */
            if( input[ inpos ] == 0 )
            {
                /* It was a single occurrence of the marker byte */
                output.push_back( marker );
                ++inpos;
            }
            else
            {
                /* Extract true length and offset */
                const unsigned length = _LZ_ReadVarSize( input, &inpos );
                const unsigned offset = _LZ_ReadVarSize( input, &inpos );

                /* Copy corresponding data from history window */
                for( unsigned i = 0; i < length; ++i )
                    output.push_back( output[ output.size() - offset ] );
            }
        }
        else
        {
            /* No marker, plain copy */
            output.push_back( symbol );
        }
    }

    EQASSERT( output.size() == totalSize );
    return inpos;
}


/*************************************************************************
 *                           INTERNAL FUNCTIONS                          *
 *************************************************************************/


/*************************************************************************
 * _LZ_StringCompare() - Return maximum length string match.
 *************************************************************************/
static unsigned int _LZ_StringCompare( const uint8_t* str1, const uint8_t* str2,
                                       const size_t minlen, const size_t maxlen)
{
    size_t len;
    for( len = minlen; (len < maxlen) && (str1[len] == str2[len]); ++ len );
    return len;
}


/*************************************************************************
 * _LZ_WriteVarSize() - Write unsigned integer with variable number of
 * bytes depending on value.
 *************************************************************************/
static void _LZ_WriteVarSize( const size_t x, vector<uint8_t>& buffer )
{
    unsigned int y;
    int num_bytes, i, b;

    /* Determine number of bytes needed to store the number x */
    y = x >> 3;
    for( num_bytes = 5; num_bytes >= 2; --num_bytes )
    {
        if( y & 0xfe000000 ) break;
        y <<= 7;
    }

    /* Write all bytes, seven bits in each, with 8:th bit set for all */
    /* but the last byte. */
    for( i = num_bytes-1; i >= 0; --i )
    {
        b = (x >> (i*7)) & 0x0000007f;
        if( i > 0 )
            b |= 0x00000080;

        buffer.push_back( static_cast<unsigned char>( b ));
    }
}


/*************************************************************************
* _LZ_ReadVarSize() - Read unsigned integer with variable number of
* bytes depending on value.
*************************************************************************/
static unsigned _LZ_ReadVarSize( const uint8_t* buffer, unsigned* position )
{

    /* Read complete value (stop when byte contains zero in 8:th bit) */
    unsigned y = 0;
    unsigned b;
    do
    {
        b = static_cast<unsigned int>(buffer[ *position ]);
        ++(*position);
        y = (y << 7) | (b & 0x0000007f);
    }
    while( b & 0x00000080 );

    /* Return value */
    return y;
}


