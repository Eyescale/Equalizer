
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/client/frustum.h>        // base class
#include <eq/client/viewport.h>       // member
#include <eq/client/visitorResult.h>  // enum
#include <eq/client/types.h>          // member

namespace eq
{
namespace server
{
    class View;
}
    class Config;
    class Layout;
    class Observer;
    class Pipe;
    class ViewVisitor;
    struct Event;

    /**
     * A View is a 2D area of a Layout. It is a view of the application's data
     * on a model, in the sense used by the MVC pattern. It can be a scene,
     * viewing mode, viewing position, or any other representation of the
     * application's data.
     */
    class View : public Frustum
    {
    public:
        EQ_EXPORT View();
        EQ_EXPORT virtual ~View();

        /** @name Data Access. */
        //@{
        /** @return the viewport of the view. */
        EQ_EXPORT const Viewport& getViewport() const;

        /** @return the config of this view. */
        EQ_EXPORT Config* getConfig();

        /** @return the config of this view. */
        EQ_EXPORT const Config* getConfig() const;

        /** @return the layout of this view. */
        EQ_EXPORT Layout* getLayout() { return _layout; }

        /** @return the layout of this view. */
        EQ_EXPORT const Layout* getLayout() const { return _layout; }

        /** @return the entity tracking this view, 0 for untracked views. */
        Observer* getObserver() { return _observer; }
        const Observer* getObserver() const { return _observer; }

        /** @warning  Undocumented - may not be supported in the future */
        EQ_EXPORT void setOverdraw( const Vector2i& pixels );
        const Vector2i& getOverdraw() const { return _overdraw; }
        //@}

        /** @name Operations */
        //@{
        /** 
         * Traverse this view using a view visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( ViewVisitor& visitor );

        /** 
         * Handle a received (view) event.
         *
         * The task of this method is to update the view as necessary. It is
         * called by Config::handleEvent on the application main thread for all
         * view events.
         * 
         * @param event the received view event.
         * @return true when the event was handled, false if not.
         */
        EQ_EXPORT virtual bool handleEvent( const Event& event );
        //@}

    protected:
        /** @sa Frustum::serialize() */
        EQ_EXPORT virtual void serialize( net::DataOStream& os,
                                          const uint64_t dirtyBits );

        /** @sa Frustum::deserialize() */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_VIEWPORT   = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_OBSERVER   = Frustum::DIRTY_CUSTOM << 1,
            DIRTY_OVERDRAW   = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 3,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 4,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 5,
        };

        /** @return the initial frustum value of this view. */
        const Frustum& getBaseFrustum() const { return _baseFrustum; }

        virtual ChangeType getChangeType() const { return DELTA; }

    private:
        /** Parent layout (application-side). */
        Layout* _layout;
        friend class Layout;

        /** Parent pipe (render-client-side). */
        Pipe* _pipe;
        friend class Pipe;

        friend class server::View;
        Viewport    _viewport;

        /** The observer for tracking. */
        Observer* _observer;

        /** Unmodified, baseline view frustum data, used when resizing. */
        Frustum _baseFrustum;

        /** Enlarge size of all dest channels and adjust frustum accordingly. */
        Vector2i _overdraw;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const View& view );
}

#endif //EQ_VIEW_H
