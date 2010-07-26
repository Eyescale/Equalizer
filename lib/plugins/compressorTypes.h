
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

#ifndef EQ_PLUGINS_COMPRESSOR_TYPES
#define EQ_PLUGINS_COMPRESSOR_TYPES

/** 
 * @file plugins/compressorTypes.h
 *
 * Compression plugin names.
 * @sa plugins/compressor.h
 */

/**
 * @name Compressor type name registry
 *
 * The compressor type registry ensures the uniqueness of compressor names. Each
 * compression engine has a unique name. It is maintained by the Equalizer
 * development team <info@equalizergraphics.com>. New types can be requested
 * free of charge.
 */
/*@{*/
// Equalizer CPU compressors
/** Invalid Compressor */
#define EQ_COMPRESSOR_INVALID               0x0u
/** No Compressor */
#define EQ_COMPRESSOR_NONE                  0x1u
/** RLE Compression of 4-byte tokens. */
#define EQ_COMPRESSOR_RLE_UNSIGNED          0x2u
/** RLE Compression of 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_BYTE              0x3u
/** RLE Compression of three 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_3_BYTE            0x4u
/** RLE Compression of four 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_4_BYTE            0x5u
/** RLE Compression of four float32 tokens. */
#define EQ_COMPRESSOR_RLE_4_FLOAT           0x6u
/** RLE Compression of four float16 tokens. */
#define EQ_COMPRESSOR_RLE_4_HALF_FLOAT      0x7u
/** Differential RLE Compression of three 1-byte tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_3_BYTE       0x8u
/** Differential RLE Compression of four 1-byte tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_4_BYTE       0x9u
/** RLE Compression of one 4-byte token. */
#define EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED   0xau
/** Lossy Differential RLE Compression. */
#define EQ_COMPRESSOR_DIFF_RLE_565          0xbu
/** RLE Compression of three token of 10-bits and one toke of 2-bits */
#define EQ_COMPRESSOR_DIFF_RLE_10A2         0xcu
/** RLE Compression of four float16 tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_4_HALF_FLOAT 0xdu
/** Differential RLE Compression of YUV tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_YUVA_50P     0xeu
/** RLE Compression of YUV tokens. */
#define EQ_COMPRESSOR_RLE_YUVA_50P          0xfu

/** Differential RLE Compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_RGBA                                 0x10u
/** Differential RLE Compression of BGRA bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_BGRA                                 0x11u
/** Differential RLE Compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_RGBA_UINT_8_8_8_8_REV                0x12u
/** Differential RLE Compression of BGRA  UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_BGRA_UINT_8_8_8_8_REV                0x13u
/** Differential RLE Compression of RGBA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_RGB10_A2                             0x14u
/** Differential RLE Compression of BGRA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_BGR10_A2                             0x15u
/** Differential RLE Compression of RGB bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_RGB                                  0x16u
/** Differential RLE Compression of BGR bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_BGR                                  0x17u
/** Differential RLE Compression of DEPTH UNSIGNED INT tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_DEPTH_UNSIGNED_INT                   0x18u
/** RLE Compression of RGBA half float tokens. */
#define EQ_COMPRESSOR_RLE_RGBA16F                                   0x19u
/** RLE Compression of BGRA half float tokens. */
#define EQ_COMPRESSOR_RLE_BGRA16F                                   0x1au
/** Differential RLE Compression of RGBA half float tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_RGBA16F                              0x1bu
/** Differential RLE Compression of BGRA half float tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_BGRA16F                              0x1cu
/** Lossy Differential RLE Compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_RGBA                             0x1du
/** Lossy Differential RLE Compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_BGRA                             0x1eu
/** Lossy Differential RLE Compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_RGBA_UINT_8_8_8_8_REV            0x1fu
/** Lossy Differential RLE Compression of BGRA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_BGRA_UINT_8_8_8_8_REV            0x20u
/** Lossy Differential RLE Compression of RGBA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_RGB10_A2                         0x21u
/** Lossy Differential RLE Compression of BGRA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_565_BGR10_A2                         0x22u
/** RLE Compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_RLE_RGBA                                      0x23u
/** RLE Compression of BGRA bytes tokens. */
#define EQ_COMPRESSOR_RLE_BGRA                                      0x24u
/** RLE Compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_RGBA_UINT_8_8_8_8_REV                     0x25u
/** RLE Compression of BGRA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_BGRA_UINT_8_8_8_8_REV                     0x26u
/** RLE Compression of RGBA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_RLE_RGB10_A2                                  0x27u
/** RLE Compression of BGRA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_RLE_BGR10_A2                                  0x28u
/** RLE Compression of RGB bytes tokens. */
#define EQ_COMPRESSOR_RLE_RGB                                       0x29u
/** RLE Compression of BGR bytes tokens. */
#define EQ_COMPRESSOR_RLE_BGR                                       0x2au
/** RLE Compression of depth unsigned int tokens. */
#define EQ_COMPRESSOR_RLE_DEPTH_UNSIGNED_INT                        0x2bu
/** RLE Compression of unsigned tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_UNSIGNED                             0x2cu

// Equalizer GPU<->CPU transfer plugins
/* Transfer data from internal RGBA to external RGBA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA                         0x100u
/* Transfer data from internal RGBA to external BGRA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA                         0x101u
/* Transfer data from internal RGBA to external RGBA format with a data type
   UNSIGNED_INT_8_8_8_8_REV */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA_UINT_8_8_8_8_REV        0x102u
/* Transfer data from internal RGBA to external BGRA format with a data type
   UNSIGNED_INT_8_8_8_8_REV */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA_UINT_8_8_8_8_REV        0x103u
/* Transfer data from internal RGB10A_2 to external RGB10A_2 format */
#define EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_RGB10A2                   0x104u
/* Transfer data from internal RGB10A_2 to external BGR10A2 format */
#define EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_BGR10A2                   0x105u
/* Transfer data from internal RGBA16F to external RGBA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA16F                   0x106u
/* Transfer data from internal RGBA16F to external BGRA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F                   0x107u
/* Transfer data from internal RGBA32F to external RGBA format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA32F                   0x108u
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F                   0x109u
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA_25P                  0x10au
/* Transfer data from internal RGBA32F to external RGBA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA_25P                  0x10bu
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA16F_50P               0x10cu
/* Transfer data from internal RGBA32F to external RGBA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA16F_50P               0x10du
/* Transfer data from internal RGBA32F to external YUVA format */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUVA_50P                     0x10eu
/* Transfer data from internal RGB to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB_TO_RGB                           0x10fu
/* Transfer data from internal RGB to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB_TO_BGR                           0x110u
/* Transfer data from internal RGB16F to external RGB format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB16F                     0x111u
/* Transfer data from internal RGB16F to external BGR format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR16F                     0x112u
/* Transfer data from internal RGB32F to external RGB format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB32F                     0x113u
/* Transfer data from internal RGB32F to external BGR format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR32F                     0x114u
/* Transfer data from internal RGB32F to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB_25P                    0x115u
/* Transfer data from internal RGB32F to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR_25P                    0x116u
/* Transfer data from internal RGB32F to external BGR format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR16F_50P                 0x117u
/* Transfer data from internal RGB32F to external RGB format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB16F_50P                 0x118u
/* Transfer data from internal RGB to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB_50P                    0x119u
/* Transfer data from internal RGB to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR_50P                    0x11au
/* Transfer data from internal RGB to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA_50P                  0x11bu
/* Transfer data from internal RGB to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA_50P                  0x11cu
/* Transfer data from internal DEPTH to external DEPTH_STENCIL */
#define EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT          0x11du
/* Transfer data from internal DEPTH_STENCIL to external DEPTH_STENCIL */
#define EQ_COMPRESSOR_TRANSFER_DEPTH_STENCIL_TO_UNSIGNED_INT_24_8   0x11eu

// Third-party plugins
/** Lossless CPU jpeg compressor from rtt.ag */
#define EQ_COMPRESSOR_AG_RTT_JPEG_HQ   0x100000u

/**
 * Private types -FOR DEVELOPMENT ONLY-.
 *
 * Any name equal or bigger than this can be used for in-house development
 * and testing. As soon as the Compressor DSO is distributed, request public
 * types free of charge from info@equalizergraphics.com.
 */
#define EQ_COMPRESSOR_PRIVATE         0xefffffffu
/*@}*/
#endif // EQ_PLUGINS_COMPRESSOR_TYPES
