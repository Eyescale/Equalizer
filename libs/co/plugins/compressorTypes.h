
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
/** RLE compression of 4-byte tokens. */
#define EQ_COMPRESSOR_RLE_UNSIGNED          0x2u
/** RLE compression of 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_BYTE              0x3u
/** RLE compression of three 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_3_BYTE            0x4u
/** RLE compression of four 1-byte tokens. */
#define EQ_COMPRESSOR_RLE_4_BYTE            0x5u
/** RLE compression of four float32 tokens. */
#define EQ_COMPRESSOR_RLE_4_FLOAT           0x6u
/** RLE compression of four float16 tokens. */
#define EQ_COMPRESSOR_RLE_4_HALF_FLOAT      0x7u
/** Differential RLE compression of three 1-byte tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_3_BYTE       0x8u
/** Differential RLE compression of four 1-byte tokens. */
#define EQ_COMPRESSOR_DIFF_RLE_4_BYTE       0x9u
/** RLE compression of one 4-byte token. */
#define EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED   0xau
/** Lossy Differential RLE compression. */
#define EQ_COMPRESSOR_RLE_DIFF_565          0xbu
/** RLE compression of three token of 10-bits and one toke of 2-bits */
#define EQ_COMPRESSOR_RLE_DIFF_BGR10_A2     0xcu
/** RLE compression of four float16 tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_4_HALF_FLOAT 0xdu
/** Differential RLE compression of YUV tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_YUVA_50P     0xeu
/** RLE compression of YUV tokens. */
#define EQ_COMPRESSOR_RLE_YUVA_50P          0xfu

/** Differential RLE compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_RGBA                                 0x10u
/** Differential RLE compression of BGRA bytes tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_BGRA                                 0x11u
/** Differential RLE compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_RGBA_UINT_8_8_8_8_REV                0x12u
/** Differential RLE compression of BGRA  UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_BGRA_UINT_8_8_8_8_REV                0x13u
/** Differential RLE compression of DEPTH UNSIGNED INT tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_DEPTH_UNSIGNED_INT                   0x16u
/** RLE compression of RGBA half float tokens. */
#define EQ_COMPRESSOR_RLE_RGBA16F                                   0x17u
/** RLE compression of BGRA half float tokens. */
#define EQ_COMPRESSOR_RLE_BGRA16F                                   0x18u
/** Differential RLE compression of RGBA half float tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_RGBA16F                              0x19u
/** Differential RLE compression of BGRA half float tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_BGRA16F                              0x1au
/** Lossy Differential RLE compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_565_RGBA                             0x1bu
/** Lossy Differential RLE compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_565_BGRA                             0x1cu
/** Lossy Differential RLE compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_565_RGBA_UINT_8_8_8_8_REV            0x1du
/** Lossy Differential RLE compression of BGRA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_565_BGRA_UINT_8_8_8_8_REV            0x1eu
/** RLE compression of RGBA bytes tokens. */
#define EQ_COMPRESSOR_RLE_RGBA                                      0x21u
/** RLE compression of BGRA bytes tokens. */
#define EQ_COMPRESSOR_RLE_BGRA                                      0x22u
/** RLE compression of RGBA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_RGBA_UINT_8_8_8_8_REV                     0x23u
/** RLE compression of BGRA UINT_8_8_8_8_REV tokens. */
#define EQ_COMPRESSOR_RLE_BGRA_UINT_8_8_8_8_REV                     0x24u
/** RLE compression of RGBA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_RLE_RGB10_A2                                  0x25u
/** RLE compression of BGRA 10_10_10_2 tokens. */
#define EQ_COMPRESSOR_RLE_BGR10_A2                                  0x26u
/** RLE compression of depth unsigned int tokens. */
#define EQ_COMPRESSOR_RLE_DEPTH_UNSIGNED_INT                        0x27u
/** RLE compression of unsigned tokens. */
#define EQ_COMPRESSOR_RLE_DIFF_UNSIGNED                             0x28u

/** LZF compression of bytes. */
#define EQ_COMPRESSOR_LZF_BYTE      0x30u
/** LZF compression of bytes. */
#define EQ_COMPRESSOR_FASTLZ_BYTE   0x31u
/** Snappy compression of bytes. */
#define EQ_COMPRESSOR_SNAPPY_BYTE   0x32u

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
/* Transfer data from internal RGBA to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGB                          0x104u
/* Transfer data from internal RGBA to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGR                          0x105u
/* Transfer data from internal RGBA32F to external YUVA format */
#define EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUVA_50P                     0x106u

/* Transfer data from internal RGB10_A2 to external BGR10_A2 format */
#define EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_BGR10_A2                 0x200u
/* Transfer data from internal RGB10_A2 to external RGB10_A2 format */
#define EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_RGB10_A2                 0x201u

/* Transfer data from internal RGBA16F to external RGBA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA16F                   0x300u
/* Transfer data from internal RGBA16F to external BGRA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F                   0x301u
/* Transfer data from internal RGBA16F to external RGB format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGB16F                    0x302u
/* Transfer data from internal RGBA16F to external BGR format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR16F                    0x303u
/* Transfer data from internal RGBA to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGB                       0x304u
/* Transfer data from internal RGBA to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR                       0x305u
/* Transfer data from internal RGBA to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA                      0x306u
/* Transfer data from internal RGBA to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA                      0x307u

/* Transfer data from internal RGBA32F to external RGBA format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA32F                   0x400u
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F                   0x401u
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA                      0x402u
/* Transfer data from internal RGBA32F to external RGBA format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA                      0x403u
/* Transfer data from internal RGBA32F to external BGRA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA16F                   0x404u
/* Transfer data from internal RGBA32F to external RGBA format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA16F                   0x405u
/* Transfer data from internal RGBA32F to external RGB format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB32F                    0x406u
/* Transfer data from internal RGBA32F to external BGR format with a data type
   float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR32F                    0x407u
/* Transfer data from internal RGBA32F to external RGB format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB                       0x408u
/* Transfer data from internal RGBA32F to external BGR format with a data type
   UNSIGNED_BYTE */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR                       0x409u
/* Transfer data from internal RGBA32F to external BGR format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR16F                    0x40au
/* Transfer data from internal RGBA32F to external RGB format with a data type
   half float */
#define EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB16F                    0x40bu

/* Transfer data from internal DEPTH to external DEPTH_STENCIL */
#define EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT          0x501u

/** Automatic selection of compressor */
#define EQ_COMPRESSOR_AUTO  0xFFFFFu

// Third-party plugins
/** Quasi-lossless CPU jpeg compressor from rtt.ag */
#define EQ_COMPRESSOR_AG_RTT_JPEG_HQ   0x100000u
/** Medium quality CPU jpeg compressor from rtt.ag */
#define EQ_COMPRESSOR_AG_RTT_JPEG_MQ   0x100001u
/** Low quality CPU jpeg compressor from rtt.ag */
#define EQ_COMPRESSOR_AG_RTT_JPEG_LQ   0x100002u
/** Quasi-lossless CPU jpeg compressor from rtt.ag retaining alpha */
#define EQ_COMPRESSOR_AG_RTT_JPEG_HQ_A   0x100003u
/** Medium quality CPU jpeg compressor from rtt.ag retaining alpha */
#define EQ_COMPRESSOR_AG_RTT_JPEG_MQ_A   0x100004u
/** Low quality CPU jpeg compressor from rtt.ag retaining alpha */
#define EQ_COMPRESSOR_AG_RTT_JPEG_LQ_A   0x100005u

/** Eyescale quasi-lossless CPU jpeg RGBA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA100 0x200000u
/** Eyescale 90% quality CPU jpeg RGBA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA90  0x200001u
/** Eyescale 80% quality CPU jpeg RGBA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGBA80  0x200002u
/** Eyescale quasi-lossless CPU jpeg BGRA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA100 0x200003u
/** Eyescale 90% quality CPU jpeg BGRA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA90  0x200004u
/** Eyescale 80% quality CPU jpeg BGRA compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGRA80  0x200005u
/** Eyescale quasi-lossless CPU jpeg RGB compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB100  0x200006u
/** Eyescale 90% quality CPU jpeg RGB compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB90   0x200007u
/** Eyescale 80% quality CPU jpeg RGB compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_RGB80   0x200008u
/** Eyescale quasi-lossless CPU jpeg BGR compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR100  0x200009u
/** Eyescale 90% quality CPU jpeg BGR compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR90   0x20000au
/** Eyescale 80% quality CPU jpeg BGR compressor */
#define EQ_COMPRESSOR_CH_EYESCALE_JPEG_BGR80   0x20000bu

/**
 * Private types -FOR DEVELOPMENT ONLY-.
 *
 * Any name equal or bigger than this can be used for in-house development
 * and testing. As soon as the Compressor DSO is distributed, request public
 * types free of charge from info@equalizergraphics.com.
 */
#define EQ_COMPRESSOR_PRIVATE           0xefffffffu

/*@}*/
#endif // EQ_PLUGINS_COMPRESSOR_TYPES
