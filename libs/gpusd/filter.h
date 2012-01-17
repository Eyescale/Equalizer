
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include <string>

namespace gpusd
{
namespace detail
{
    class Filter;
    class SessionFilter;
}
    /** Base class for all discovery filters. */
    class Filter
    {
    public:
        /** Create a new filter. @version 1.0 */
        GPUSD_API Filter();

        /** Destruct this filter. @version 1.0 */
        GPUSD_API virtual ~Filter();

        /**
         * Chain another filter to this one.
         *
         * Invoking the operator() will call chained filters.
         * @version 1.0
         */
        GPUSD_API FilterPtr operator | ( FilterPtr rhs );

        /**
         * Chain another filter to this one.
         *
         * Invoking the operator() will call chained filters.
         * @version 1.0
         */
        GPUSD_API FilterPtr operator |= ( FilterPtr rhs );

        /**
         * Call all chained operators.
         *
         * Filter implementations overwrite this method to implement the
         * filtering and call this base class implementation if the candidate
         * passed.
         *
         * @param current the list of passed GPU informations.
         * @param candidate the new GPU information to test.
         * @return true if all chained operators returned true, false otherwise.
         * @version 1.0
         */
        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate );
    private:
        detail::Filter* const impl_;
    };

    /** Filters all duplicates during discovery. */
    class DuplicateFilter : public Filter
    {
    public:
        virtual ~DuplicateFilter() {}

        /**
         * @return true if the candidate is not in the current vector.
         * @version 1.0
         */
        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate );
    };

    /** Filter overlapping duplicates with different GPU types. */
    class MirrorFilter : public Filter
    {
    public:
        virtual ~MirrorFilter() {}

        /**
         * @return true if the candidate is unique wrt the position, device,
         *         hostname and session.
         * @version 1.0
         */
        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate );
    };

    /** Filters for a specific session. */
    class SessionFilter : public Filter
    {
    public:
        GPUSD_API SessionFilter( const std::string& name );
        GPUSD_API virtual ~SessionFilter();

        /** @return true if the candidate has the given session. @version 1.0 */
        GPUSD_API virtual bool operator() ( const GPUInfos& current,
                                            const GPUInfo& candidate );
    private:
        detail::SessionFilter* const impl_;
    };
}
#endif // GPUSD_FILTER_H

