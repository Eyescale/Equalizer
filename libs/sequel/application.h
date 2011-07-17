
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include <sequel/objectFactory.h> // interface
#include <sequel/types.h>
#include <eq/client/client.h>               // base class

namespace seq
{
    /** The main application object. */
    class Application : public eq::Client, public seq::ObjectFactory
    {
    public:
        /** Construct a new application instance. @version 1.0 */
        SEQ_API Application();

        /** Destruct this application instance. @version 1.0 */
        SEQ_API virtual ~Application();

        /** @name Data Access. */
        //@{
        SEQ_API eq::Config* getConfig(); //!< @internal
        detail::Application* getImpl() { return _impl; } //!< @internal

        /**
         * Create a new renderer instance.
         *
         * Called once per rendering thread, potentially in parallel, during
         * initialization.
         * @return the new renderer
         * @version 1.0
         */
        virtual Renderer* createRenderer() = 0;

        /** Delete the given renderer. @version 1.0 */
        SEQ_API virtual void destroyRenderer( Renderer* renderer );

        /**
         * Create a new per-view data instance.
         *
         * Called once for each view in the current configuration.
         * @return the new view data
         * @version 1.0
         */
        SEQ_API virtual ViewData* createViewData();

        /** Delete the given view data. @version 1.0 */
        SEQ_API virtual void destroyViewData( ViewData* viewData );
        //@}

        /** @name Operations */
        //@{
        /** 
         * Initialize the application instance.
         *
         * The initData object is registered and is passed to all initialization
         * callbacks on all processes. The object may be 0, if the application
         * does not want to use an object during initialization.
         *
         * @param argc the command line argument count.
         * @param argv the command line arguments.
         * @param initData a distributable object for initialization data.
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool init( const int argc, char** argv,
                                   co::Object* initData );
        
        /**
         * Run the application main loop.
         *
         * The frameData object is registered and is passed to all rendering
         * callbacks on all processes. It is automatically committed at the
         * beginning of each frame. The instance passed to the render callbacks
         * is automatically synchronized to the version belonging to the frame
         * rendered. The object may be 0, if the application does not want to
         * use a per-frame object.
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
        virtual bool clientInit( co::Object* initData ) { return true; }

        /** Exit a render client. @version 1.0 */
        virtual bool clientExit() { return true; }
        //@}

    private:
        detail::Application* _impl;
    };
}
#endif // EQSEQUEL_APPLICATION_H
