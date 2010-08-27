
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

#ifndef EQBASE_COMPRESSORINFO_H
#define EQBASE_COMPRESSORINFO_H

#include <eq/base/types.h>
#include <eq/plugins/compressor.h> // base struct

namespace eq
{
namespace base
{

/** @internal Augment the plugin information with additional data. */
struct CompressorInfo : public EqCompressorInfo
{
    CompressorInfoPtrs compressors; //!< potential compressors of a downloader
    CompressorInfoPtrs uploaders;   //!< potential uploaders of a decompressor
};


//EQ_EXPORT std::ostream& operator << ( std::ostream&, const CompressorInfo& );

}
}

#endif //EQBASE_COMPRESSORINFO_H
