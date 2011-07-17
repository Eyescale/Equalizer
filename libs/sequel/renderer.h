
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

#include <sequel/objectFactory.h> // interface
#include <sequel/types.h>

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
        co::Object* getFrameData(); // @warning experimental

        /** @return the application instance for this renderer. @version 1.0 */
        Application& getApplication() { return _app; }

        /** @return the application instance for this renderer. @version 1.0 */
        const Application& getApplication() const { return _app; }

        /**
         * Create a new per-view data instance.
         *
         * Called once for each view used by this renderer.
         * @return the new view data
         * @version 1.0
         */
        SEQ_API virtual ViewData* createViewData();

        /** Delete the given view data. @version 1.0 */
        SEQ_API virtual void destroyViewData( ViewData* viewData );

        /** 
         * Get the GLEW context for this renderer.
         * 
         * The glew context provides access to OpenGL extensions. This function
         * does not follow the Sequel naming conventions, since GLEW uses a
         * function of this name to automatically resolve OpenGL function entry
         * points. Therefore, any OpenGL function support by the driver can be
         * directly called from any method of an initialized renderer.
         * 
         * @return the extended OpenGL function table for the window's OpenGL
         *         context.
         * @version 1.0
         */
        SEQ_API const GLEWContext* glewGetContext() const;

        /** @return the current view frustum. @version 1.0 */
        SEQ_API const Frustumf& getFrustum() const;

        /** @return the current view (frustum) transformation. @version 1.0 */
        SEQ_API const Matrix4f& getViewMatrix() const;

        /** @return the current model (scene) transformation. @version 1.0 */
        SEQ_API const Matrix4f& getModelMatrix() const;
        //@}

        /** @name Operations */
        //@{
        /** 
         * Initialize the OpenGL context.
         *
         * Called after a context has been created and made current.
         * @param initData a per-renderer instance of the object passed to
         *                 Config::init().
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool initGL( co::Object* initData );

        /** 
         * Deinitialize the OpenGL context.
         *
         * Called just before the context will be destroyed.
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool exitGL();

        /**
         * Render the scene.
         *
         * @param frameData the renderer instance of the object passed to
         *                  Config::run.
         * @version 1.0
         */
        virtual void draw( co::Object* frameData ) = 0;

        /**
         * Apply the current rendering parameters to OpenGL.
         *
         * This method sets the draw buffer, color mask, viewport as well as the
         * projection and view matrix.
         *
         * This method is only to be called from clear(), draw() and TBD.
         * @version 1.0
         */
        SEQ_API virtual void applyRenderContext();

        /**
         * Apply the current model matrix to OpenGL.
         *
         * This method is not included in applyRenderContext() since ligthing
         * parameters are often applied before positioning the model.
         *
         * This method is only to be called from clear(), draw() and TBD.
         * @version 1.0
         */
        SEQ_API virtual void applyModelMatrix();
        //@}

        /** @name ObjectFactory interface, forwards to Application instance. */
        //@{
        SEQ_API virtual eq::Config* getConfig();
        SEQ_API virtual co::Object* createObject( const uint32_t type );
        SEQ_API virtual void destroyObject( co::Object* object,
                                            const uint32_t type );
        //@}

    private:
        detail::Renderer* _impl;
        Application& _app;
    };
}
#endif // EQSEQUEL_RENDERER_H
