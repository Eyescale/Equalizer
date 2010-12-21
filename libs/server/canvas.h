
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_CANVAS_H
#define EQSERVER_CANVAS_H

#include "types.h"
#include "visitorResult.h"  // enum

#include <eq/fabric/canvas.h> // base class

#include <co/base/os.h>
#include <string>

namespace eq
{
namespace server
{
    /** The canvas. @sa eq::Canvas */
    class Canvas : public fabric::Canvas< Config, Canvas, Segment, Layout >
    {
    public:
        /** 
         * Constructs a new Canvas.
         */
        EQSERVER_EXPORT Canvas( Config* parent );

        /** Destruct this canvas. */
        virtual ~Canvas();

        /**
         * @name Data Access
         */
        //@{
        /** @return the Server of this canvas. @version 1.0 */
        ServerPtr getServer();

        /** @return the segment of the given path. */
        Segment* getSegment( const SegmentPath& path );

        /** @return the index path to this canvas. @internal */
        CanvasPath getPath() const;

        /** @return true if this canvas is initialized. */
        bool isStopped() const { return _state == STATE_STOPPED; }

        /** @return true if this canvas is initialized. */
        bool isRunning() const { return _state == STATE_RUNNING; }

        /** @return true if this canvas should be deleted. */
        bool needsDelete() const { return _state == STATE_DELETE; }
        //@}

        /**
         * @name Operations
         */
        //@{
        void init();
        void exit();

        /** Schedule deletion of this canvas. */
        void postDelete();
        //@}
        
    protected:
        virtual void activateLayout( const uint32_t index );

    private:
        enum State
        {
            STATE_STOPPED = 0,  // next: RUNNING
            STATE_RUNNING,      // next: STOPPED or DELETE
            STATE_DELETE,       // next: destructor
        }
            _state;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** Run-time layout switch */
        void _switchLayout( const uint32_t oldIndex, const uint32_t newIndex );

    };

}
}
#endif // EQSERVER_CANVAS_H
