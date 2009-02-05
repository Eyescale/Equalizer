
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CANVAS_H
#define EQSERVER_CANVAS_H

#include "types.h"
#include "visitorResult.h"  // enum

#include <eq/client/canvas.h>

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
    class Canvas : public eq::Canvas
    {
    public:
        /** 
         * Constructs a new Canvas.
         */
        Canvas();

        /** Creates a new, deep copy of a canvas. */
        Canvas( const Canvas& from, Config* config );

        /** Destruct this canvas. */
        virtual ~Canvas();

        /**
         * @name Data Access
         */
        //*{
        /** Add a new segment to this canvas. */
        void addSegment( Segment* segment );
        
        /** Get the list of segments. */
        const SegmentVector& getSegments() const { return _segments; }

        /** @return the currently used layout. */
        Layout* getLayout() { return _layout; }
        /** @return the currently used layout. */
        const Layout* getLayout() const { return _layout; }
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
        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The currently active layout on this canvas. */
        Layout* _layout;

        /** Child segments on this canvas. */
        SegmentVector _segments;
    };

    std::ostream& operator << ( std::ostream& os, const Canvas* canvas);
}
}
#endif // EQSERVER_CANVAS_H
