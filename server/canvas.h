
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    class Canvas : public eq::Frustum
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
        void init();
        void exit();

        void useLayout( Layout* layout );

        /** 
         * Traverse this canvas and all children using a canvas visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( CanvasVisitor& visitor );
        VisitorResult accept( ConstCanvasVisitor& visitor ) const;

        /** Unmap this canvas and all its children. */
        void unmap();
        //*}
        
    protected:
        /** @sa Frustum::serialize */
        virtual void serialize( net::DataOStream& os, 
                                const uint64_t dirtyBits );

        /** @sa Frustum::deserialize */
        virtual void deserialize( net::DataIStream& is, 
                                  const uint64_t dirtyBits );

    private:
        virtual void getInstanceData( net::DataOStream& os );

        /** The parent config. */
        Config* _config;
        friend class Config;

        /** The currently active layout on this canvas. */
        Layout* _layout;

        /** Child segments on this canvas. */
        SegmentVector _segments;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Run-time layout */
        void _switchLayout( const Layout* oldLayout, const Layout* newLayout );
    };

    std::ostream& operator << ( std::ostream& os, const Canvas* canvas);
}
}
#endif // EQSERVER_CANVAS_H
