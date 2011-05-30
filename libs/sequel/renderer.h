
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

#ifndef EQSEQUEL_RENDERER_H
#define EQSEQUEL_RENDERER_H

#include <eq/sequel/objectFactory.h> // interface
#include <eq/sequel/types.h>

namespace seq
{
    /**
     * A renderer instance.
     *
     * All calls to one renderer instance are guaranteed to be executed from a
     * single thread.
     */
    class Renderer : public ObjectFactory
    {
    public:
        /** Construct a new renderer. @version 1.0 */
        SEQ_API Renderer( Application& application );

        /** Destruct this renderer. @version 1.0 */
        SEQ_API virtual ~Renderer();

        /** @name Data Access. */
        //@{
        detail::Renderer* getImpl() { return _impl; } //!< @internal
        //@}

        /** @name Operations */
        //@{
        /** Render the scene. @version 1.0 */
        virtual void draw() = 0;
        //@}

        /** @name ObjectFactory interface, forwards to Application instance. */
        //@{
        virtual eq::Config* getConfig();
        virtual co::Object* createObject( const uint32_t type );
        virtual void destroyObject( co::Object* object, const uint32_t type );
        //@}

    private:
        detail::Renderer* _impl;
        Application& _app;
    };
}
#endif // EQSEQUEL_RENDERER_H
