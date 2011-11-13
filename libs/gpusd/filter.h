
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

#ifndef GPUSD_FILTER_H
#define GPUSD_FILTER_H

#include <gpusd/api.h>
#include <gpusd/types.h>
#include <algorithm>
#include <string>

namespace gpusd
{
    /** Base class for all discovery filters. */
    class Filter
    {
    public:
        Filter() : next_( 0 ) {}
        virtual ~Filter() {}

        Filter& operator | ( const Filter& rhs ) { next_ = &rhs; return *this; }
        virtual bool operator() ( const GPUInfos& current,
                                  const GPUInfo& candidate ) const
            { return next_ ? (*next_)( current, candidate ) : true; }

    private:
        const Filter* next_;
    };

    /** Filters all duplicates during discovery. */
    class DuplicateFilter : public Filter
    {
    public:
        virtual ~DuplicateFilter() {}

        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate ) const;
    };

    /** Filters for a specific session. */
    class SessionFilter : public Filter
    {
    public:
        SessionFilter( const std::string name ) : name_( name ) {}
        virtual ~SessionFilter() {}

        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate ) const;
    private:
        const std::string& name_;
    };
}
#endif // GPUSD_FILTER_H

