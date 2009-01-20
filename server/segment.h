
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_SEGMENT_H
#define EQSERVER_SEGMENT_H

#include "types.h"
#include "segmentVisitor.h"  // used in inline method

#include <eq/client/frustum.h>      // base class
#include <eq/client/projection.h>   // member
#include <eq/client/viewport.h>     // member
#include <eq/client/wall.h>         // member

#include <string>

namespace eq
{
namespace server
{
    class Canvas;

    /**
     * The segment. @sa eq::Segment
     */
    class Segment : public eq::Frustum
    {
    public:
        /** 
         * Constructs a new Segment.
         */
        Segment() : _canvas( 0 ), _channel( 0 ) {}

        /** Creates a new, deep copy of a segment. */
        Segment( const Segment& from, Config* config );

        /** Destruct this segment. */
        virtual ~Segment();

        /**
         * @name Data Access
         */
        //*{
        /** Set the name of this segment. */
        void setName( const std::string& name ) { _name = name; }

        /** @return the name of this segment. */
        const std::string& getName() const      { return _name; }

        /** 
         * Set the channel of this segment.
         *
         * The channel defines the output area for this segment, typically a
         * rendering area covering a graphics card output.
         * 
         * @param channel the channel.
         */
        void setChannel( Channel* channel ) { _channel = channel; }

        /** Return the output channel of this segment. */
        Channel* getChannel()               { return _channel; }
        const Channel* getChannel() const   { return _channel; }

        /** 
         * Set the segment's viewport wrt its canvas.
         *
         * The viewport defines which 2D area of the canvas is covered by this
         * segment. Destination channels are created on the intersection of
         * segment viewports and the views of the layout used by the canvas.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp ) { _vp = vp; }

        /** @return the segment's viewport. */
        const eq::Viewport& getViewport() const { return _vp; }
        //*}
        
        /** Operations */
        //*{
        /** 
         * Traverse this segment using a segment visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( SegmentVisitor* visitor )
            { return visitor->visit( this ); }
        //*}

    protected:

    private:
        /** The parent canvas. */
        Canvas* _canvas;

        /** The output channel of this segment. */
        Channel* _channel;

        /** The name of this segment. */
        std::string _name;

        /** The 2D area of this segment wrt to the canvas. */
        eq::Viewport _vp;

        /** Update the view (wall/projection). */
        void _updateView();
    };

    std::ostream& operator << ( std::ostream& os, const Segment* segment);
}
}
#endif // EQSERVER_SEGMENT_H
