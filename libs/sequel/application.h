
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

#include <eq/sequel/objectFactory.h> // interface
#include <eq/sequel/types.h>
#include <eq/client.h>               // base class

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
        virtual eq::Config* getConfig(); //!< @internal

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
        virtual void destroyRenderer( Renderer* renderer );
        //@}

        /** @name Operations */
        //@{
        /** 
         * Initialize the application instance.
         *
         * The initData object is registered and can be later accessed from any
         * context using getObject( OBJECTTYPE_INITDATA ). The object may be 0,
         * if the application does not want to use an object during
         * initialization.
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
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool run();

        /**
         * Exit this application instance.
         *
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool exit();
        //@}

    private:
        detail::Application* _impl;
    };
}
#endif // EQSEQUEL_APPLICATION_H
