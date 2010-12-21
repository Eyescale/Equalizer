
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

/** 
 * @file plugins/compressorTokens.h
 *
 * Input and output token type definitions for compression plugins.
 * @sa plugins/compressor.h
 */

#ifndef EQ_PLUGINS_COMPRESSOR_TOKENS
#define EQ_PLUGINS_COMPRESSOR_TOKENS

/**
 * @name Compressor token types
 *
 * The input and output compressor token types are reported by the DSO, and
 * define which type of data can be processed by the given compressor. It is
 * used by Equalizer to select candidates for compression. The output token type
 * is only used for transfer plugins.
 */
/*@{*/
/** Invalid data. */
#define EQ_COMPRESSOR_DATATYPE_NONE       0x0
/** Data is processed in one-byte tokens. */
#define EQ_COMPRESSOR_DATATYPE_BYTE       0x1
/** Data is processed in four-byte tokens. */
#define EQ_COMPRESSOR_DATATYPE_UNSIGNED   0x2
/** Data is processed in float16 tokens. */
#define EQ_COMPRESSOR_DATATYPE_HALF_FLOAT 0x3
/** Data is processed in float32 tokens. */
#define EQ_COMPRESSOR_DATATYPE_FLOAT      0x4

/** Data is processed in three interleaved streams of one-byte tokens. */
#define EQ_COMPRESSOR_DATATYPE_3_BYTE       0x400
/** Data is processed in four interleaved streams of one-byte tokens. */
#define EQ_COMPRESSOR_DATATYPE_4_BYTE       0x401
/** Data is processed in four interleaved streams of float16 tokens. */
#define EQ_COMPRESSOR_DATATYPE_3_HALF_FLOAT 0x402
/** Data is processed in four interleaved streams of float16 tokens. */
#define EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT 0x403
/** Data is processed in four interleaved streams of three float32 tokens.*/
#define EQ_COMPRESSOR_DATATYPE_3_FLOAT      0x404
/** Data is processed in four interleaved streams of four float32 tokens. */
#define EQ_COMPRESSOR_DATATYPE_4_FLOAT      0x405

/**Data is processed in one 24 bit and one 8 bit interleaved streams. */
#define EQ_COMPRESSOR_DATATYPE_3BYTE_1BYTE  0x800

/** Data is processed in three 10-bit color tokens and one 2-bit alpha token. */
#define EQ_COMPRESSOR_DATATYPE_BGR10_A2     0x801

/**
 * Data is processed in four interleaved streams of RGBA color of unsigned Byte
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGBA                           0X1908
/**
 * Data is processed in four interleaved streams of RGBA color of unsigned byte
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV          0x1000
/**
 * Data is processed in four interleaved streams of RGBA color of half float
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGBA16F                        0x881a
/**
 * Data is processed in four interleaved streams of RGBA color of float tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGBA32F                        0x8814
/**
 * Data is processed in four interleaved streams of BGRA color of unsigned byte
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGRA    EQ_COMPRESSOR_DATATYPE_4_BYTE
/**
 * Data is processed in four interleaved streams of BGRA color of unsigned byte
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV          0x1001
/**
 * Data is processed in four interleaved streams of BGRA color of half float
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGRA16F EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT
/**
 * Data is processed in four interleaved streams of BGRA color of float tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGRA32F EQ_COMPRESSOR_DATATYPE_4_FLOAT

/** Data is a (source) depth buffer. */
#define EQ_COMPRESSOR_DATATYPE_DEPTH                          0x1902
/** Data is processed in one stream of depth of float tokens. */
#define EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT                    0x1003
/** Data is processed in one stream of unsigned int depth tokens. */
#define EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT EQ_COMPRESSOR_DATATYPE_UNSIGNED

/**
 * Data is processed in three interleaved streams of RGB unsigned byte color
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGB                            0x1907
/**
 * Data is processed in three interleaved streams of RGB half float color
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_RGB16F                         0x881b
/** Data is processed in three interleaved streams of RGB float color tokens. */
#define EQ_COMPRESSOR_DATATYPE_RGB32F                         0x8815
/**
 * Data is processed in three interleaved streams of BGR unsigned byte color
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGR EQ_COMPRESSOR_DATATYPE_3_BYTE
/**
 * Data is processed in three interleaved streams of BGR half float color
 * tokens.
 */
#define EQ_COMPRESSOR_DATATYPE_BGR16F EQ_COMPRESSOR_DATATYPE_3_HALF_FLOAT
/** Data is processed in three interleaved streams of BGR float color tokens. */
#define EQ_COMPRESSOR_DATATYPE_BGR32F EQ_COMPRESSOR_DATATYPE_3_FLOAT

/**
 * Data is processed in four interleaved streams of YUVA components. 
 * Special image format reducing color sampling.
 */
#define EQ_COMPRESSOR_DATATYPE_YUVA_50P                       0x1004

/** Data is processed in three 10-bit color tokens and one 2-bit alpha token. */
#define EQ_COMPRESSOR_DATATYPE_RGB10_A2     0x8059

/**
 * Invalid data type.
 * Used by a plugin to disable incompatible engines at runtime in
 * EqCompressorGetInfo().
 */
#define EQ_COMPRESSOR_DATATYPE_INVALID      0xeffffffeu

/**
 * Private token types -FOR DEVELOPMENT ONLY-.
 *
 * Any token type equal or bigger than this can be used for in-house development
 * and testing. As soon as the Compressor DSO is distributed, request public
 * types free of charge from info@equalizergraphics.com.
 */
#define EQ_COMPRESSOR_DATATYPE_PRIVATE      0xefffffffu
/*@}*/

#endif // EQ_PLUGINS_COMPRESSOR_TOKENS
