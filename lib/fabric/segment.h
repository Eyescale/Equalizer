
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_SEGMENT_H
#define EQFABRIC_SEGMENT_H

#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h>  // enum
#include <eq/fabric/frustum.h>        // base class
#include <eq/fabric/object.h>         // base class
#include <eq/fabric/viewport.h>       // member

namespace eq
{
namespace fabric
{
    /**
     * A segment covers a sub-area of a Canvas. It has a Frustum, and defines
     * one output Channel of the whole projection area, typically a projector or
     * screen.
     */
    template< class C, class S, class CH >
    class Segment : public Object, public Frustum
    {
    public:
        typedef LeafVisitor< S > Visitor;

        /** @name Data Access */
        //@{
        /** @return the parent canvas. @version 1.0 */
        const C* getCanvas() const { return _canvas; }

        /** @return the parent canvas. @version 1.0 */
        C* getCanvas() { return _canvas; }

        /** @return the segment's viewport. @version 1.0 */
        const Viewport& getViewport() const { return _vp; }

        /** 
         * Set the segment's viewport wrt its canvas.
         *
         * The viewport defines which 2D area of the canvas is covered by this
         * segment. Destination channels are created on the intersection of
         * segment viewports and the views of the layout used by the canvas.
         * 
         * @param vp the fractional viewport.
         * @internal
         */
        EQFABRIC_EXPORT void setViewport( const Viewport& vp );

        /** 
         * Set the channel of this segment.
         *
         * The channel defines the output area for this segment, typically a
         * rendering area covering a graphics card output.
         * 
         * @param channel the channel.
         * @internal
         */
        void setChannel( CH* channel )
            { _channel = channel; setDirty( DIRTY_CHANNEL ); }

        /** Return the output channel of this segment. */
        CH* getChannel()               { return _channel; }

        /** Return the output channel of this segment. */
        const CH* getChannel() const   { return _channel; }

        /** @sa Frustum::setWall() */
        EQFABRIC_EXPORT virtual void setWall( const Wall& wall );
        
        /** @sa Frustum::setProjection() */
        EQFABRIC_EXPORT virtual void setProjection( const Projection& );

        /** @sa Frustum::unsetFrustum() */
        EQFABRIC_EXPORT virtual void unsetFrustum();

        /** @return the bitwise OR of the eye values. */
        uint32_t getEyes() const { return _eyes; }

         /** 
         * Set the eyes to be used by the segument.
         * 
         * Previously set eyes are overwritten.
         *
         * @param eyes the segment eyes.
         */
        EQFABRIC_EXPORT void setEyes( const uint32_t eyes );

        /** 
         * Add eyes to be used by the segument.
         *
         * Previously set eyes are preserved.
         * 
         * @param eyes the segument eyes.
         */
        void enableEye( const uint32_t eyes ) { _eyes |= eyes; }
        //@}
        
        /** @name Operations */
        //@{
        /** 
         * Traverse this segment using a segment visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( Visitor& visitor ) const;

        /**
         * Notify that a condition affecting the frustum has changed. @internal
         */
        void notifyFrustumChanged();

        virtual void backup(); //!< @internal
        virtual void restore(); //!< @internal
        //@}

    protected:
        /** Construct a new Segment. */
        EQFABRIC_EXPORT Segment( C* canvas );

        /** Destruct this segment. */
        EQFABRIC_EXPORT virtual ~Segment();

        /** @sa Frustum::serialize */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os, 
                                                const uint64_t dirtyBits );
        /** @sa Frustum::deserialize */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        /** @sa Serializable::setDirty() @internal */
        virtual void setDirty( const uint64_t bits );

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        enum DirtyBits
        {
            DIRTY_VIEWPORT   = Object::DIRTY_CUSTOM << 0,
            DIRTY_FRUSTUM    = Object::DIRTY_CUSTOM << 1,
            DIRTY_CHANNEL    = Object::DIRTY_CUSTOM << 2,
            DIRTY_EYES       = Object::DIRTY_CUSTOM << 3
        };

        /** The parent canvas. */
        C* const _canvas;

        /** The 2D area of this segment wrt to the canvas. */
        Viewport _vp;

        /** The output channel of this segment. */
        CH* _channel;

        uint32_t _eyes;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        virtual uint32_t commitNB(); //!< @internal
    };

    template< class C, class S, class CH >
    std::ostream& operator << ( std::ostream&, const Segment< C, S, CH >& );
}
}
#endif // EQFABRIC_SEGMENT_H
