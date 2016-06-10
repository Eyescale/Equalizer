
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSEQUEL_RENDERER_H
#define EQSEQUEL_RENDERER_H

#include <co/objectFactory.h> // interface
#include <seq/types.h>

namespace seq
{
/**
 * A renderer instance.
 *
 * All calls to one renderer instance are guaranteed to be executed from a
 * single thread.
 */
class Renderer : public co::ObjectFactory
{
public:
    /** Construct a new renderer. @version 1.0 */
    SEQ_API explicit Renderer( Application& application );

    /** Destruct this renderer. @version 1.0 */
    SEQ_API virtual ~Renderer();

    /** @name Operations */
    //@{
    /**
     * Initialize the renderer.
     *
     * Called once per renderer with a rendering context current before the
     * first call to initContext().
     *
     * @param initData a per-renderer instance of the object passed to
     *                 Config::init().
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool init( co::Object* initData LB_UNUSED )
        { return true; }

    /**
     * De-initialize the renderer.
     *
     * Called just before the last context will be destroyed after the last call
     * to exitContext().
     *
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool exit() { return true; }

    /**
     * Initialize a rendering context.
     *
     * Called for each window handled by this renderer, after the context has
     * been created and made current.
     *
     * @param initData a per-renderer instance of the object passed to
     *                 Config::init().
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool initContext( co::Object* initData );

    /**
     * De-initialize a rendering context.
     *
     * Called just before the context will be destroyed.
     *
     * @return true on success, false otherwise.
     * @version 1.0
     */
    SEQ_API virtual bool exitContext();

    /**
     * Clear the frame buffer.
     *
     * @param frameData the renderer's instance of the object passed to
     *                  Config::run.
     * @version 1.0
     */
    SEQ_API virtual void clear( co::Object* frameData );

    /**
     * Render the scene.
     *
     * @param frameData the renderer's instance of the object passed to
     *                  Config::run.
     * @version 1.0
     */
    virtual void draw( co::Object* frameData ) = 0;

    /** Request another call to draw(). @version 1.12 */
    SEQ_API void requestRedraw();

    /**
     * Update the near and far planes to tightly enclose the given sphere.
     *
     * @param boundingSphere the sphere bounding of the scene to be rendered.
     * @version 1.7.1
     */
    SEQ_API void updateNearFar( const Vector4f& boundingSphere );

    /**
     * Set the near and far planes.
     *
     * The given near and far planes update the current perspective and
     * orthographics frustum accordingly.
     *
     * @param nearPlane the near plane.
     * @param farPlane the far plane.
     * @version 1.7.1
     */
     SEQ_API void setNearFar( const float nearPlane, const float farPlane );

    /**
     * Apply the current rendering parameters to the rendering context.
     *
     * This method sets the draw buffer, color mask, viewport as well as the
     * projection and view matrix.
     *
     * This method is only to be called from clear(), draw() and TBD.
     * @version 1.0
     */
    SEQ_API virtual void applyRenderContext();

    /**
     * Bind the window draw buffer, which can be (multisampled) FBO or window
     * buffer.
     * @version 1.12
     */
    SEQ_API void bindDrawFrameBuffer();

    /** @return the current rendering parameters. @version 1.4 */
    SEQ_API const RenderContext& getRenderContext() const;

    /**
     * Apply the current model matrix to the rendering context.
     *
     * This method is not included in applyRenderContext() since ligthing
     * parameters are often applied before positioning the model.
     *
     * This method is only to be called from clear(), draw() and TBD.
     * @version 1.0
     */
    SEQ_API virtual void applyModelMatrix();

    /**
     * Apply an orthographic frustum for pixel-based 2D operations.
     *
     * One unit of the frustum covers one pixel on screen. The frustum is
     * positioned relative to the eq::View.
     * @version 1.8
     */
    SEQ_API virtual void applyScreenFrustum();

    /**
     * Apply the perspective frustum matrix for the current rendering task.
     * @version 1.8
     */
    SEQ_API virtual void applyPerspectiveFrustum();

    /**
     * @warning experimental
     * @return true when the event was handled, false if not.
    */
    SEQ_API virtual bool processEvent( const eq::Event& ) { return false; }
    //@}

    /** @name Data Access */
    //@{
    detail::Renderer* getImpl() { return _impl; } //!< @internal

    /**
     * @return A frame-synchronous instance of the data passed to
     *         Application::run().
     * @version 1.8
     */
    SEQ_API co::Object* getFrameData();

    /** @return the application instance for this renderer. @version 1.0 */
    Application& getApplication() { return app_; }

    /** @return the application instance for this renderer. @version 1.0 */
    const Application& getApplication() const { return app_; }

    /** @return the object manager of this renderer. @version 1.0 */
    SEQ_API const ObjectManager& getObjectManager() const;

    /** @return the object manager of this renderer. @version 1.0 */
    SEQ_API ObjectManager& getObjectManager();

    /**
     * Create a new per-view data instance.
     *
     * Called once for each view used by this renderer. Creates the view
     * instance used by the renderer to retrieve parameters from the
     * application for rendering.
     *
     * @param view the view requesting the view data
     * @return the new view data
     * @version 1.11
     */
    SEQ_API virtual ViewData* createViewData( View& view );

    /** Delete the given view data. @version 1.0 */
    SEQ_API virtual void destroyViewData( ViewData* viewData );

    /** @return the current view data. @version 1.12 */
    SEQ_API const ViewData* getViewData() const;

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

    /** @return the current rendering area. @version 1.8 */
    SEQ_API const PixelViewport& getPixelViewport() const;

    /**
     * @return a unique, stable identifier for the current window.
     * @version 1.8
     */
    SEQ_API uint32_t getWindowID() const;
    //@}

    /** @name Distributed Object API */
    //@{
    /** @sa seq::Application::createObject() */
    SEQ_API virtual co::Object* createObject( const uint32_t type );

    /** @sa seq::Application::destroyObject() */
    SEQ_API virtual void destroyObject( co::Object* object,
                                        const uint32_t type );

    /**
     * Map and return an object.
     *
     * @param identifier unique object identifier used for map operation
     * @param instance already created instance to skip factory creation
     * @return 0 if not registered, the valid instance otherwise
     * @version 1.8
     * @sa co::ObjectMap::map()
     */
    SEQ_API co::Object* mapObject( const uint128_t& identifier,
                                   co::Object* instance );

    /**
     * Unmap an object from the object map.
     *
     * @param object the object to unmap
     * @return false on if object was not mapped, true otherwise
     * @version 1.0
     * @sa co::ObjectMap::unmap()
     */
    SEQ_API bool unmap( co::Object* object );
    //@}

private:
    detail::Renderer* const _impl;
    Application& app_;
};
}
#endif // EQSEQUEL_RENDERER_H
