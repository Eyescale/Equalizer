
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_COMPRESSOR_H
#define EQBASE_COMPRESSOR_H

#include <vector>

namespace eqBase
{
    /**
     * Static functions to do data compression
     */
    class Compressor 
    {
    public:
        /** 
         * Compress the input data using LZ compression.
         * 
         * @param input The input data.
         * @param output vector where output data will be appended.
         */
        static void compressLZ( const std::vector<uint8_t>& input, 
                                 std::vector<uint8_t>& output );

        /** 
         * Decompress the LZ-compressed input data.
         * 
         * @param input The input data.
         * @param output vector where output data will be appended.
         * @return the input data size.
         */
        static uint32_t Compressor::decompressLZ( const uint8_t* input,
                                                  std::vector<uint8_t>& output);
    private:
        Compressor();
        ~Compressor();
    };
}

#endif //EQBASE_COMPRESSOR_H
