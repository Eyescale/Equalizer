
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SEGMENT_H
#define EQ_SEGMENT_H

#include <eq/client/frustum.h>        // base class
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

namespace eq
{
namespace server
{
    class Segment;
}

    class Canvas;
    class SegmentVisitor;

    /**
     * A Segment covers a sub-area of a canvas. It has a frustum, and defines
     * one output channel of the whole frustum, typically a projector or screen.
     */
    class Segment : public eq::Frustum
    {
    public:
        /** 
         * Constructs a new Segment.
         */
        Segment();

        /** Destruct this segment. */
        virtual ~Segment();

        /**
         * @name Data Access
         */
        //*{
        /** @return the config of this view. */
        Config* getConfig();

        /** @return the config of this view. */
        const Config* getConfig() const;

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
        VisitorResult accept( SegmentVisitor* visitor );
        //*}

    protected:
        /** @sa Frustum::serialize */
        EQ_EXPORT virtual void serialize( net::DataOStream& os, 
                                          const uint64_t dirtyBits );
        /** @sa Frustum::deserialize */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_VIEWPORT   = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 1,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 3
        };

    private:
        /** The parent canvas. */
        Canvas* _canvas;
        friend class Canvas;

        /** The 2D area of this segment wrt to the canvas. */
        eq::Viewport _vp;
        friend class server::Segment;
    };

}
#endif // EQ_SEGMENT_H
