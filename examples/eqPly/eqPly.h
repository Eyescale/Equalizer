
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

#ifndef EQ_PLY_H
#define EQ_PLY_H

#include <eq/eq.h>

#include "vertexBufferDist.h"
#include "vertexBufferRoot.h"


/** The Equalizer polygonal rendering example. */
namespace eqPly
{
    class LocalInitData;

    typedef mesh::VertexBufferRoot    Model;
    typedef VertexBufferDist          ModelDist;

    /** The EqPly application instance */
    class EqPly : public eq::Client
    {
    public:
        EqPly( const LocalInitData& initData );
        virtual ~EqPly() {}

        /** Run an eqPly instance. */
        int run();

        static const std::string& getHelp();

    protected:
        /** @sa eq::Client::clientLoop. */
        virtual bool clientLoop();
        
    private:
        const LocalInitData& _initData;
    };

    enum ColorMode
    {
        COLOR_MODEL, //!< Render using the colors defined in the ply file
        COLOR_DEMO,  //!< Use a unique color to demonstrate decomposition
        COLOR_WHITE, //!< Render in solid white (mostly for anaglyph stereo)
        COLOR_ALL    //!< @internal, must be last
    };

    enum LogTopics
    {
        LOG_STATS = eq::LOG_CUSTOM << 0, // 65536
        LOG_CULL  = eq::LOG_CUSTOM << 1  // 131072
    };
}

#endif // EQ_PLY_H

