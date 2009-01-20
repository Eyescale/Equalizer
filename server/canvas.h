
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CANVAS_H
#define EQSERVER_CANVAS_H

#include "types.h"
#include "viewData.h"       // member
#include "visitorResult.h"  // enum

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace server
{
    class CanvasVisitor;

    /**
     * The canvas. @sa eq::Canvas
     */
    class Canvas
    {
    public:
        /** 
         * Constructs a new Canvas.
         */
        Canvas() : _layout( 0 ) {}

        /** Creates a new, deep copy of a canvas. */
        Canvas( const Canvas& from, Config* config );

        /** Destruct this canvas. */
        virtual ~Canvas();

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Set the name of this canvas.
         *
         * The names is used by the canvas referenc canvass in the config file.
         */
        void setName( const std::string& name ) { _name = name; }

        /** @return the name of this canvas. */
        const std::string& getName() const      { return _name; }

        /** Add a new segment to this canvas. */
        void addSegment( Segment* segment ) { _segments.push_back( segment ); }
        
        /** Get the list of segments. */
        const SegmentVector& getSegments() const { return _segments; }

        /** Set the view using a wall description. */
        void setWall( const eq::Wall& wall );
        
        /** Set the view using a projection description. */
        void setProjection( const eq::Projection& projection );
        //*}

        /**
         * @name Operations
         */
        //*{
        void useLayout( Layout* layout );

        /** 
         * Traverse this canvas and all children using a canvas visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( CanvasVisitor* visitor );
        //*}
        
    protected:

    private:
        /** The name of this canvas. */
        std::string _name;

        /** The currently active layout on this canvas. */
        Layout* _layout;

        /** Child segments on this canvas. */
        SegmentVector _segments;

        /** Frustum information. */
        ViewData _data;

        /** Update the view (wall/projection). */
        void _updateView();
    };

    std::ostream& operator << ( std::ostream& os, const Canvas* canvas);
}
}
#endif // EQSERVER_CANVAS_H
