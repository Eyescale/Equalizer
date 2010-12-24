
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef COBASE_COMPRESSORINFO_H
#define COBASE_COMPRESSORINFO_H

#include <co/base/types.h>
#include <co/plugins/compressor.h> // base struct

#include <iostream>

namespace co
{
namespace base
{

/** @internal Augment the plugin information with additional data. */
struct CompressorInfo : public EqCompressorInfo
{
    CompressorInfoPtrs compressors; //!< potential compressors of a downloader
    CompressorInfoPtrs uploaders;   //!< potential uploaders of a decompressor
};


inline std::ostream& operator << ( std::ostream& os, const CompressorInfo& info)
{
    os << "v" << info.version << std::hex << " name 0x" << info.name
       << " in 0x" << info.tokenType << " out 0x" << info.outputTokenType
       << " cap 0x" << info.capabilities << std::dec << " quality "
       << info.quality << " ratio " << info.ratio << " speed " << info.speed;
    return os;
}

}
}

#endif //COBASE_COMPRESSORINFO_H
