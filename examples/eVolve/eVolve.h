
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EVOLVE_H
#define EVOLVE_H

#include <eq/eq.h>

#include "rawVolModelRenderer.h"


/** The Equalizer Volume Rendering Example */
namespace eVolve
{
    class LocalInitData;
    typedef RawVolumeModelRenderer Renderer;


    class EVolve : public eq::Client
    {
    public:
        EVolve( const LocalInitData& initData );
        virtual ~EVolve() {}

        /** Run an eqPly instance. */
        int run();

        static const std::string& getHelp();

    protected:
        /** @sa eq::Client::clientLoop. */
        virtual bool clientLoop();
        
    private:
        const LocalInitData& _initData;
    };

    enum LogTopics
    {
        LOG_STATS = eq::LOG_CUSTOM      // 65536
    };
}

#endif // EVOLVE_H

