
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/visitorResult.h>  // enum
#include <eq/types.h>          // member
#include <eq/api.h>

#include <eq/fabric/view.h>           // base class
#include <eq/fabric/viewport.h>       // member

namespace eq
{
    struct Event;

    /**
     * A View is a 2D area of a Layout. It is a view of the application's data
     * on a model, in the sense used by the MVC pattern. It can be a scene,
     * viewing mode, viewing position, or any other representation of the
     * application's data.
     *
     * @warning Never commit a View. Equalizer does take care of this to
     *          correctly associate view version with rendering frames.
     * @sa fabric::View
     */
    class View : public fabric::View< Layout, View, Observer >
    {
    public:
        /** Construct a new view. @version 1.0 */
        EQ_API View( Layout* parent );

        /** Destruct this view. @version 1.0 */
        EQ_API virtual ~View();

        /** @name Data Access. */
        //@{
        /** @return the config of this view. @version 1.0 */
        EQ_API Config* getConfig();

        /** @return the config of this view. @version 1.0 */
        EQ_API const Config* getConfig() const;

        /** @return the Server of this view. @version 1.0 */
        EQ_API ServerPtr getServer();
        //@}

        /** @name Operations */
        //@{
        /** 
         * Handle a received (view) event.
         *
         * The task of this method is to update the view as necessary. It is
         * called by Config::handleEvent on the application main thread for all
         * view events.
         * 
         * @param event the received view event.
         * @return true when the event was handled, false if not.
         * @version 1.0
         */
        EQ_API virtual bool handleEvent( const Event& event );
        //@}

    protected:
        /** @internal */
        EQ_API virtual void deserialize( co::DataIStream& is, 
                                         const uint64_t dirtyBits );

        /** @return the initial frustum value of this view. */
        const Frustum& getBaseFrustum() const { return _baseFrustum; }

    private:
        Pipe* _pipe; // for render-client views
        friend class Pipe;

        /** Unmodified, baseline view frustum data, used when resizing. */
        Frustum _baseFrustum;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes
    };
}

#endif //EQ_VIEW_H
