
/* Copyright (c) 2013, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_TRANSFERFINDER_H
#define EQ_TRANSFERFINDER_H

#include <pression/plugin.h>
#include <pression/pluginVisitor.h>
#include <pression/plugins/compressor.h>

namespace eq
{
namespace
{
class TransferFinder : public pression::ConstPluginVisitor
{
public:
    TransferFinder( const uint32_t internal, const uint32_t external,
                    const uint64_t caps, const float minQuality,
                    const bool ignoreAlpha, const GLEWContext* gl )
        : internal_( internal )
        , external_( external )
        , caps_( caps | EQ_COMPRESSOR_TRANSFER )
        , minQuality_( minQuality )
        , ignoreAlpha_( ignoreAlpha )
        , gl_( gl )
    {}

    virtual ~TransferFinder() {}

    virtual fabric::VisitorResult visit( const pression::Plugin& plugin,
                                         const EqCompressorInfo& info )
    {
        if(( (info.capabilities & caps_) == caps_ )                &&
           ( internal_ == EQ_COMPRESSOR_DATATYPE_NONE ||
             info.tokenType == internal_ )                         &&
           ( external_ == EQ_COMPRESSOR_DATATYPE_NONE ||
             info.outputTokenType == external_ )                   &&
           ( info.quality >= minQuality_ )                         &&
           ( ignoreAlpha_ ||
             !(info.capabilities & EQ_COMPRESSOR_IGNORE_ALPHA ))   &&
           ( !gl_ || plugin.isCompatible( info.name, gl_ )))
        {
            result.push_back( info );
        }
        return fabric::TRAVERSE_CONTINUE;
    }

    EqCompressorInfos result;

private:
    const uint32_t internal_;
    const uint32_t external_;
    const uint64_t caps_;
    const float minQuality_;
    const bool ignoreAlpha_;
    const GLEWContext* gl_;
};
}
}
#endif
