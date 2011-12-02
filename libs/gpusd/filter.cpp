
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "filter.h"
#include <gpusd/gpuInfo.h>

namespace gpusd
{
/** @cond IGNORE */
typedef std::vector< FilterPtr > Filters;
typedef Filters::const_iterator FiltersCIter;
/** @endcond */

// Filter
//-------
namespace detail
{
class Filter
{
public:
    Filters next_;
};
}

Filter::Filter() : impl_( new detail::Filter ) {}
Filter::~Filter() { delete impl_; }

bool Filter::operator() ( const GPUInfos& current,
                          const GPUInfo& candidate )
{
    for( FiltersCIter i = impl_->next_.begin(); i!=impl_->next_.end(); ++i)
    {
        FilterPtr filter = *i;
        if( !(*filter)( current, candidate ))
            return false;
    }
    return true;
}

FilterPtr Filter::operator | ( FilterPtr rhs )
{
    impl_->next_.push_back( rhs );
    return rhs;
}

FilterPtr Filter::operator |= ( FilterPtr rhs )
{
    impl_->next_.push_back( rhs );
    return rhs;
}

// DuplicateFilter
//----------------
bool DuplicateFilter::operator() ( const GPUInfos& current,
                                   const GPUInfo& candidate )
{
    if( std::find( current.begin(), current.end(), candidate ) == current.end())
        return Filter::operator()( current, candidate );
    return false;
}

// MirrorFilter
//-------------
bool MirrorFilter::operator() ( const GPUInfos& current,
                                const GPUInfo& candidate )
{
    for( GPUInfosCIter i = current.begin(); i != current.end(); ++i )
    {
        const GPUInfo& info = *i;
        if( info.hostname == candidate.hostname &&
            info.session == candidate.session &&
            info.device == candidate.device &&
            info.pvp[0] == candidate.pvp[0] && info.pvp[1] == candidate.pvp[1] )
        {
             return false;
        }
    }

    return Filter::operator()( current, candidate );
}

// SessionFilter
//--------------
namespace detail
{
class SessionFilter
{
public:
    SessionFilter( const std::string& name ) : name_( name ) {}

    const std::string& name_;
};
}

SessionFilter::SessionFilter( const std::string& name )
        : impl_( new detail::SessionFilter( name ))
{}

SessionFilter::~SessionFilter() { delete impl_; }

bool SessionFilter::operator() ( const GPUInfos& current,
                                 const GPUInfo& candidate )
{
    if( candidate.session == impl_->name_ )
        return Filter::operator()( current, candidate );
    return false;
}

}
