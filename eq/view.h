
/* Copyright (c) 2008-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/api.h>
#include <eq/types.h>          // member
#include <eq/frame.h>          // enum
#include <eq/visitorResult.h>  // enum

#include <eq/fabric/view.h>           // base class
#include <eq/fabric/viewport.h>       // member

namespace eq
{
namespace detail { class View; }

/**
 * A View is a 2D area of a Layout. It is a view of the application's data on a
 * model, in the sense used by the MVC pattern. It can be a scene, viewing mode,
 * viewing position, or any other representation of the application's data.
 *
 * @warning Never commit a View. Equalizer does take care of this to correctly
 *          associate view version with rendering frames.
 * @sa fabric::View
 */
class View : public fabric::View< Layout, View, Observer >
{
public:
    /** Construct a new view. @version 1.0 */
    EQ_API explicit  View( Layout* parent );

    /** Destruct this view. @version 1.0 */
    EQ_API virtual ~View();

    /** @name Data Access. */
    //@{
    /**
     * @return the parent pipe of this view, 0 for non-render client views.
     * @version 1.1.2
     */
    Pipe* getPipe() { return _pipe; }

    /**
     * @return the parent pipe of this view, 0 for non-render client views.
     * @version 1.1.2
     */
    EQ_API const Pipe* getPipe() const { return _pipe; }

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
     * view events. Thread safe.
     *
     * @param type the event type.
     * @param event the received view event.
     * @return true when the event was handled, false if not.
     * @version 1.0
     */
    EQ_API virtual bool handleEvent( EventType type, const SizeEvent& event );

    /**
     * Callback function called during eq::Config::handleEvents() after
     * capturing a complete screenshot.
     *
     * @version 2.1
     */
    using ScreenshotFunc = std::function< void( uint32_t, const Image& ) >;

    /**
     * Enable recording of given buffers for screenshot feature.
     *
     * @param buffers bitmask of buffers to capture in screenshot image
     * @param func callback function with frame number and screenshot image
     * @version 2.1
     */
    EQ_API void enableScreenshot( Frame::Buffer buffers,
                                  const ScreenshotFunc& func );

    /** Stop recording of screenshots. @version 2.1 */
    EQ_API void disableScreenshot();

    /** @internal */
    bool handleEvent( EventICommand& command );

    /** @internal */
    void sendScreenshotEvent( const Viewport& viewport,
                              const uint32_t frameNumber, const Image& image );
    //@}

protected:
    /**
     * @name Callbacks
     *
     * Callbacks are called by Equalizer during rendering to execute various
     * actions from the application main thread before sending the corresponding
     * command to the server.
     */
    //@{
    /** Initialize this view. @version 1.11 */
    virtual bool configInit() { return true; }
    friend class detail::InitVisitor;

    /** Exit this view. @version 1.11 */
    virtual bool configExit() { return true; }
    friend class detail::ExitVisitor;
    //@}

    /** @internal */
    EQ_API virtual void deserialize( co::DataIStream& is,
                                     const uint64_t dirtyBits );

    /** @return the initial frustum value of this view. */
    EQ_API const Frustum& getBaseFrustum() const;

    /** @internal trigger deletion for render-client views. */
    EQ_API virtual void detach();

private:
    detail::View* const _impl;

    bool _handleScreenshot( EventICommand& command );

    Pipe* _pipe; // for render-client views
    friend class Pipe;
};
}

#endif //EQ_VIEW_H
