
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CANVAS_H
#define EQ_CANVAS_H

#include <eq/client/frustum.h>        // base class
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

#include <eq/net/object.h>
#include <string>

namespace eq
{
    class CanvasVisitor;

    /**
     * The canvas.
     *
     * The canvas is a logical 2D projection surface. The server has a list of
     * segments on the canvas, which are mapped to physical output
     * channels. Segments are not visible to the application. The canvas uses a
     * Layout to define the set of logical views used to render on the
     * canvas. The layout can be switched at runtime. A canvas without a layout
     * does not render anything.
     */
    class Canvas : public Frustum
    {
    public:
        /** 
         * Constructs a new Canvas.
         */
        EQ_EXPORT Canvas();

        /** Destruct this canvas. */
        EQ_EXPORT virtual ~Canvas();

        /**
         * @name Data Access
         */
        //*{
        /** @return the parent config. */
        Config*       getConfig()       { return _config; }
        /** @return the parent config. */
        const Config* getConfig() const { return _config; }

        /** @return the layout used by the canvas. */
        Layout*       getLayout()       { return _layout; }
        /** @return the layout used by the canvas. */
        const Layout* getLayout() const { return _layout; }

        /** @return the vector of child segments. */
        const SegmentVector& getSegments() const { return _segments; }        
        //*}

        /**
         * @name Operations
         */
        //*{
        EQ_EXPORT void useLayout( Layout* layout );

        /** 
         * Traverse this canvas and all children using a canvas visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( CanvasVisitor* visitor );

        /** Deregister this canvas, and all children, from its net::Session.*/
        EQ_EXPORT virtual void deregister();
        //*}

    protected:
        /** @sa Frustum::deserialize */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_LAYOUT     = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 1,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 3
        };

    private:
        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The currently active layout on this canvas. */
        Layout* _layout;

        /** Child segments on this canvas. */
        SegmentVector _segments;
    };
}
#endif // EQ_CANVAS_H
