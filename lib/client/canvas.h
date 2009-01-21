
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CANVAS_H
#define EQ_CANVAS_H

#include <eq/client/frustum.h>        // base class
#include <eq/client/visitorResult.h>  // enum

#include <eq/net/object.h>
#include <string>

namespace eq
{
namespace server
{
    class Canvas;
}
    class CanvasVisitor;
    class Config;
    class Layout;

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
        /** @return the name of this canvas. */
        EQ_EXPORT const std::string& getName() const      { return _name; }
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
        //*}

    protected:
        /** @sa Frustum::serialize */
        EQ_EXPORT virtual void serialize( net::DataOStream& os, const uint32_t dirtyBits);
        /** @sa Frustum::deserialize */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                  const uint32_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_LAYOUT     = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 1
        };

    private:
        friend class eq::server::Canvas;

        /** The name of this canvas. */
        std::string _name;

        /** The parent config. */
        Config* _config;

        /** The currently active layout on this canvas. */
        Layout* _layout;
    };
}
#endif // EQ_CANVAS_H
