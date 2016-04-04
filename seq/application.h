
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Petros Kataras <petroskataras@gmail.com>
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

#ifndef EQSEQUEL_APPLICATION_H
#define EQSEQUEL_APPLICATION_H

#include <co/objectFactory.h> // interface
#include <seq/types.h>
#include <eq/client.h>               // base class

namespace seq
{
/** The main application object. */
class Application : public eq::Client, public co::ObjectFactory
{
public:
    /** Construct a new application instance. @version 1.0 */
    SEQ_API Application();

    /** @name Data Access */
    //@{
    /** @return the node running the main instance. @version 1.3.1 */
    SEQ_API co::NodePtr getMasterNode();
    //@}

    /** @name Operations */
    //@{
    /**
     * Initialize the application instance.
     *
     * The initData object is registered and is passed to all initialization
     * callbacks on all processes. The object may be 0, if the application does
     * not want to use an object during initialization.
     *
     * @param argc the command line argument count.
     * @param argv the command line arguments.
     * @param initData a distributable object for initialization data.
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool init( int argc, char** argv, co::Object* initData );

    /**
     * Run the application main loop.
     *
     * The frameData object is registered and is passed to all rendering
     * callbacks on all processes. It is automatically committed at the
     * beginning of each frame. The instance passed to the render callbacks is
     * automatically synchronized to the version belonging to the frame
     * rendered. The object may be 0, if the application does not want to use a
     * per-frame object.
     *
     * @return true on success, false otherwise.
     * @param frameData a distributed object holding frame-specific data.
     * @version 1.0
     */
    SEQ_API virtual bool run( co::Object* frameData );

    /**
     * Exit this application instance.
     *
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool exit();

    /** Request that the application leaves its run loop. @version 1.1.6 */
    SEQ_API void stopRunning();
    //@}

    /** @name Callbacks */
    //@{
    /**
     * Initialize a render client.
     *
     * Also called on the master application node if it contributes to the
     * rendering.
     *
     * @param initData A slave instance of the object passed to init().
     * @return true on success, false on error.
     * @version 1.0
     */
    virtual bool clientInit( co::Object* initData LB_UNUSED )
        { return true; }

    /** Exit a render client. @version 1.0 */
    virtual bool clientExit() { return true; }

    /**
     * Create a new renderer instance.
     *
     * Called once per rendering thread, potentially in parallel, during
     * initialization.
     *
     * @return the new renderer
     * @version 1.0
     */
    virtual Renderer* createRenderer() = 0;

    /** Delete the given renderer. @version 1.0 */
    SEQ_API virtual void destroyRenderer( Renderer* renderer );

    /**
     * Create a new per-view data instance.
     *
     * Called once for each view in the current configuration. Creates the view
     * data objects used by the application to set parameters for the renderers.
     *
     * @param view the view requesting the view data
     * @return the new view data
     * @version 1.11
     */
    SEQ_API virtual ViewData* createViewData( View& view );

    /** Delete the given view data. @version 1.0 */
    SEQ_API virtual void destroyViewData( ViewData* viewData );
    //@}

    /** @name Internal */
    //@{
    SEQ_API eq::Config* getConfig(); //!< @internal
    detail::Application* getImpl() { return _impl; } //!< @internal
    //@}

    /** @name Distributed Object API */
    //@{
    /**
     * Add and register a new object as master instance.
     *
     * @param object the new object to add and register
     * @param type unique object type to create object via slave factory
     * @return true on success, false otherwise.
     * @version 1.8
     * @sa co::ObjectMap::register_()
     */
    SEQ_API bool registerObject( co::Object* object, const uint32_t type );

    /**
     * Remove and deregister an object.
     *
     * @param object the object to remove and deregister
     * @return false if object was not registered, true otherwise
     * @version 1.8
     * @sa co::ObjectMap::deregister()
     */
    SEQ_API bool deregister( co::Object* object );
    //@}

protected:
    /** Destruct this application instance. @version 1.0 */
    SEQ_API virtual ~Application();

private:
    detail::Application* _impl;
};
}
#endif // EQSEQUEL_APPLICATION_H
