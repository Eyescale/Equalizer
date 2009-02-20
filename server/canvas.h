
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
    class ConstCanvasVisitor;
    struct CanvasPath;
    struct SegmentPath;

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
        Config* getConfig()             { return _config; }
        const Config* getConfig() const { return _config; }

        /** Add a new segment to this canvas. */
        void addSegment( Segment* segment );
        
        /** @return the vector of child segments. */
        const SegmentVector& getSegments() const { return _segments; }

        /** 
         * Find the first segment of a given name.
         * 
         * @param name the name of the segment to find
         * @return the first segment with the name, or <code>0</code> if no
         *         segment with the name exists.
         */
        Segment* findSegment( const std::string& name );

        /** @return the currently used layout. */
        Layout* getLayout() { return _layout; }
        /** @return the currently used layout. */
        const Layout* getLayout() const { return _layout; }

        /** @return the segment of the given path. */
        Segment* getSegment( const SegmentPath& path );

        /** @return the index path to this canvas. */
        CanvasPath getPath() const;
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
        VisitorResult accept( ConstCanvasVisitor* visitor ) const;

        /** Unmap this canvas and all its children. */
        void unmap();
        //*}
        
    protected:
        /** @sa Frustum::serialize */
        virtual void serialize( net::DataOStream& os, 
                                          const uint64_t dirtyBits );

    private:
        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The currently active layout on this canvas. */
        Layout* _layout;

        /** Child segments on this canvas. */
        SegmentVector _segments;

        // WAR to set the layout on client and server during deserialize
        virtual void _setLayout( const uint32_t id );
    };

    std::ostream& operator << ( std::ostream& os, const Canvas* canvas);
}
}
#endif // EQSERVER_CANVAS_H
