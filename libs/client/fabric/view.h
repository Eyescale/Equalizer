
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_VIEW_H
#define EQFABRIC_VIEW_H

#include <eq/fabric/api.h>
#include <eq/fabric/frustum.h>        // base class
#include <eq/fabric/object.h>         // base class
#include <eq/fabric/types.h>
#include <eq/fabric/viewport.h>       // member
#include <eq/fabric/visitorResult.h>  // enum

namespace eq
{
namespace fabric
{
    /** Base data transport class for views. @sa eq::View */
    template< class L, class V, class O >
    class View : public Object, public Frustum
    {
    public:
        /** The current rendering mode. */
        enum Mode
        {
            MODE_MONO = 1,   //!< Render in mono (cyclop eye)
            MODE_STEREO      //!< Render in stereo (left & right eye)
        };

        /** @name Data Access. */
        //@{
        /** @return the viewport of the view wrt its layout. @version 1.0 */
        EQFABRIC_INL const Viewport& getViewport() const;

        /** @return the parent layout of this view, can be 0. @version 1.0 */
        L* getLayout() { return _layout; }

        /** @return the parent layout of this view, can be 0. @version 1.0 */
        const L* getLayout() const { return _layout; }

        /**
         * @return the observer tracking this view, or 0 for untracked views.
         * @version 1.0
         */
        O* getObserver() { return _observer; }

        /** const version of getObserver(). @version 1.0 */
        const O* getObserver() const { return _observer; }

        /** @sa Frustum::setWall() @version 1.0 */
        EQFABRIC_INL virtual void setWall( const Wall& wall );
        
        /** @sa Frustum::setProjection() @version 1.0 */
        EQFABRIC_INL virtual void setProjection( const Projection& );

        /** @sa Frustum::unsetFrustum() @version 1.0 */
        EQFABRIC_INL virtual void unsetFrustum();

        /** @warning  Undocumented - may not be supported in the future */
        EQFABRIC_INL void setOverdraw( const Vector2i& pixels );

        /** @warning  Undocumented - may not be supported in the future */
        const Vector2i& getOverdraw() const { return _overdraw; }

        /** @internal Set the 2D viewport wrt Layout and Canvas. */
        EQFABRIC_INL void setViewport( const Viewport& viewport );

        /** @internal Set the entity tracking this view. */
        EQFABRIC_INL void setObserver( O* observer );
        
        /** @internal Get the mode of this view. */
        Mode getMode( ) const { return _mode; }
        
        /**
         * Set the mode of this view.
         *
         * @param mode the new rendering mode
         * @version 1.0
         */
        EQFABRIC_INL virtual void changeMode( const Mode mode );
        
        /**
         * @internal
         * Activate the given mode on this view.
         *
         * @param mode the new rendering mode 
         */
        virtual void activateMode( const Mode mode ){ _mode = mode; }
        //@}

        /** @name Operations */
        //@{
        /** 
         * Traverse this view using a view visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_INL VisitorResult accept( LeafVisitor< V >& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_INL VisitorResult accept( LeafVisitor< V >& visitor ) const;

        virtual EQFABRIC_INL void backup(); //!< @internal
        virtual EQFABRIC_INL void restore(); //!< @internal
        //@}

        /**
         * Set the minimum required capabilities for this view.
         *
         * Any channel which does not support all of the bits in this mask does
         * not execute any tasks. By default no bit is set.
         * 
         * @param bitmask the capabilities as bitmask
         * @version 1.0
         */
        EQFABRIC_INL void setMinimumCapabilities( const uint64_t bitmask );

        /** @return the bitmask of the minimum capabilities. @version 1.0 */
        EQFABRIC_INL uint64_t getMinimumCapabilities() const;
        //@}

        /**
         * Set the maximum desired capabilities for this view.
         *
         * The capabilities returned by getCapabilities() during rendering match
         * the lowest common denominator of all channel capabilities and this
         * bitmask. Logically it has to be a superset of the minimum
         * capabilities. By default all bits are set.
         *
         * The capabilities are used to selectively disable source channels in
         * conjunction with a load equalizer. Each channel typically sets its
         * capabilities during configInit. The application sets the minimum
         * and maximum capabilities needed or desired to render this view. The
         * channel queries the capabilities to be used using getCapabilities().
         *
         * @param bitmask the capabilities as bitmask
         * @version 1.0 
         */
        EQFABRIC_INL void setMaximumCapabilities(const uint64_t bitmask);

        /**
         * @return the bitmask that represents the maximum capabilities.
         * @version 1.0
         */
        EQFABRIC_INL uint64_t getMaximumCapabilities() const;
        //@}

        /**
         * @return the bitmask usable for rendering.
         * @sa setMaximumCapabilities()
         * @version 1.0
         */
        EQFABRIC_INL uint64_t getCapabilities() const;
        //@}

        void setCapabilities( const uint64_t bitmask ); //!< @internal
        virtual void updateCapabilities() {}; //!< @internal

    protected:
        /** @internal Construct a new view. */
        EQFABRIC_INL View( L* layout );

        /** @internal Destruct this view. */
        EQFABRIC_INL virtual ~View();

        /**
         * By default, the application view instance holds the user data master.
         * @sa Object::hasMasterUserData().
         * @version 1.0
         */
        virtual bool hasMasterUserData() { return getLayout() != 0; }

        /** @internal Set number of kept versions to latency. */
        EQFABRIC_INL virtual void notifyAttached();

        /** @internal */
        EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                             const uint64_t dirtyBits );

        /** @internal */
        EQFABRIC_INL virtual void deserialize( co::DataIStream& is, 
                                                  const uint64_t dirtyBits );

        /** @internal */
        EQFABRIC_INL virtual void setDirty( const uint64_t bits );

        /** @internal */
        enum DirtyBits
        {
            DIRTY_VIEWPORT      = Object::DIRTY_CUSTOM << 0,
            DIRTY_OBSERVER      = Object::DIRTY_CUSTOM << 1,
            DIRTY_OVERDRAW      = Object::DIRTY_CUSTOM << 2,
            DIRTY_FRUSTUM       = Object::DIRTY_CUSTOM << 3,
            DIRTY_MODE          = Object::DIRTY_CUSTOM << 4,
            DIRTY_MINCAPS       = Object::DIRTY_CUSTOM << 5,
            DIRTY_MAXCAPS       = Object::DIRTY_CUSTOM << 6,
            DIRTY_CAPABILITIES  = Object::DIRTY_CUSTOM << 7,
            DIRTY_VIEW_BITS =
                DIRTY_VIEWPORT | DIRTY_OBSERVER | DIRTY_OVERDRAW |
                DIRTY_FRUSTUM | DIRTY_MODE | DIRTY_MINCAPS | DIRTY_MAXCAPS |
                DIRTY_CAPABILITIES | DIRTY_OBJECT_BITS
        };

        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_VIEW_BITS; }

    private:
        /** Parent layout (application-side). */
        L* const _layout;

        /** The observer for tracking. */
        O* _observer;

        /** Logical 2D area of Canvas covered. */
        Viewport _viewport;

        /** Enlarge size of all dest channels and adjust frustum accordingly. */
        Vector2i _overdraw;

        Mode _mode;

        uint64_t _minimumCapabilities;
        uint64_t _maximumCapabilities;
        uint64_t _capabilities;  

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };

    template< class L, class V, class O >
    EQFABRIC_INL std::ostream& operator << ( std::ostream& os,
                                             const View< L, V, O >& view );
}
}
#endif // EQFABRIC_VIEW_H
