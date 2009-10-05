
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

#ifndef EVOLVE_PIPE_H
#define EVOLVE_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

namespace eVolve
{
    class Pipe : public eq::Pipe
    {
    public:
        Pipe( eq::Node* parent ) : eq::Pipe( parent ) {}

        const FrameData& getFrameData() const { return _frameData; }

        Renderer*        getRenderer()        { return _renderer;  }
        const Renderer*  getRenderer() const  { return _renderer;  }

    protected:
        virtual ~Pipe() {}

        virtual eq::WindowSystem selectWindowSystem() const;
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

    private:
        FrameData _frameData;

        Renderer*   _renderer;      //!< The renderer, holding the volume
    };
}

#endif // EVOLVE_PIPE_H
