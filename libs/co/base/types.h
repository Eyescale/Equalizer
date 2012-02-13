
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * @file base/types.h
 *
 * Basic type definitions not provided by the operating system.
 */

#ifndef COBASE_TYPES_H
#define COBASE_TYPES_H

#include <string>
#include <vector>
#include <sys/types.h>

#ifndef _MSC_VER
#  include <stdint.h>
#endif

#ifdef _WIN32
#  include <basetsd.h>

typedef int        socklen_t;

#  ifdef _MSC_VER
typedef UINT64     uint64_t;
typedef INT64      int64_t;
typedef UINT32     uint32_t;
typedef INT32      int32_t;
typedef UINT16     uint16_t;
typedef INT16      int16_t;
typedef UINT8      uint8_t;
typedef INT8       int8_t;
#    ifndef HAVE_SSIZE_T
typedef SSIZE_T    ssize_t;
#    endif
#  endif // Win32, Visual C++
#endif // Win32

#define EQ_BIT1  (0x00000001u)
#define EQ_BIT2  (0x00000002u)
#define EQ_BIT3  (0x00000004u)
#define EQ_BIT4  (0x00000008u)
#define EQ_BIT5  (0x00000010u)
#define EQ_BIT6  (0x00000020u)
#define EQ_BIT7  (0x00000040u)
#define EQ_BIT8  (0x00000080u)

#define EQ_BIT9  (0x00000100u)
#define EQ_BIT10 (0x00000200u)
#define EQ_BIT11 (0x00000400u)
#define EQ_BIT12 (0x00000800u)
#define EQ_BIT13 (0x00001000u)
#define EQ_BIT14 (0x00002000u)
#define EQ_BIT15 (0x00004000u)
#define EQ_BIT16 (0x00008000u)

#define EQ_BIT17 (0x00010000u)
#define EQ_BIT18 (0x00020000u)
#define EQ_BIT19 (0x00040000u)
#define EQ_BIT20 (0x00080000u)
#define EQ_BIT21 (0x00100000u)
#define EQ_BIT22 (0x00200000u)
#define EQ_BIT23 (0x00400000u)
#define EQ_BIT24 (0x00800000u)

#define EQ_BIT25 (0x01000000u)
#define EQ_BIT26 (0x02000000u)
#define EQ_BIT27 (0x04000000u)
#define EQ_BIT28 (0x08000000u)
#define EQ_BIT29 (0x10000000u)
#define EQ_BIT30 (0x20000000u)
#define EQ_BIT31 (0x40000000u)
#define EQ_BIT32 (0x80000000u)

#define EQ_BIT33 (0x0000000100000000ull)
#define EQ_BIT34 (0x0000000200000000ull)
#define EQ_BIT35 (0x0000000400000000ull)
#define EQ_BIT36 (0x0000000800000000ull)
#define EQ_BIT37 (0x0000001000000000ull)
#define EQ_BIT38 (0x0000002000000000ull)
#define EQ_BIT39 (0x0000004000000000ull)
#define EQ_BIT40 (0x0000008000000000ull)

#define EQ_BIT41 (0x0000010000000000ull)
#define EQ_BIT42 (0x0000020000000000ull)
#define EQ_BIT43 (0x0000040000000000ull)
#define EQ_BIT44 (0x0000080000000000ull)
#define EQ_BIT45 (0x0000100000000000ull)
#define EQ_BIT46 (0x0000200000000000ull)
#define EQ_BIT47 (0x0000400000000000ull)
#define EQ_BIT48 (0x0000800000000000ull)

#define EQ_BIT_ALL_32  (0xffffffffu)
#define EQ_BIT_ALL_64  (0xffffffffffffffffull)
#define EQ_BIT_NONE (0)

#define EQ_1KB   (1024)
#define EQ_10KB  (10240)
#define EQ_100KB (102400)
#define EQ_1MB   (1048576)
#define EQ_10MB  (10485760)
#define EQ_100MB (104857600)

#define EQ_16KB  (16384)
#define EQ_32KB  (32768)
#define EQ_64KB  (65536)
#define EQ_128KB (131072)
#define EQ_48MB  (50331648)
#define EQ_64MB  (67108864)

namespace co
{
namespace base
{
/** A vector of std::strings @version 1.0 */
typedef std::vector< std::string >   Strings;
typedef Strings::const_iterator StringsCIter;

class CPUCompressor; //!< @internal
class Plugin;        //!< @internal
class ErrorRegistry;
class PluginRegistry;
class SpinLock;
class UUID;
struct CompressorInfo; //!< @internal

/** @internal A vector of compressor information structures. */
typedef std::vector< CompressorInfo > CompressorInfos;

/** @internal A vector of pointers to compressor information. */
typedef std::vector< const CompressorInfo* > CompressorInfoPtrs;

/** @internal A vector of compression DSO interfaces. */
typedef std::vector< Plugin* > Plugins;

}
}

#endif //COBASE_TYPES_H
