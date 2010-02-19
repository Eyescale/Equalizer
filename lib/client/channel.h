
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include <eq/client/event.h>         // member
#include <eq/client/renderContext.h> // member
#include <eq/client/types.h>
#include <eq/client/visitorResult.h> // enum
#include <eq/client/windowSystem.h>  // GLEWContext
#include <eq/client/window.h>

#include <eq/net/object.h>           // base class

namespace eq
{
namespace util
{
    class FrameBufferObject;
}
    class ChannelVisitor;
    class Pixel;
    class Range;
    class SceneObject;
    struct RenderContext;

    /**
     * A channel represents a two-dimensional viewport within a Window.
     *
     * The channel is the basic rendering entity. It represents a 2D rendering
     * area within a Window. It executes all rendering-relevant tasks, such as
     * clear, draw, assemble and readback. Each rendering task is using its own
     * RenderContext, which is computed by the server based on the rendering
     * description of the current configuration.
     */
    class Channel : public net::Object
    {
    public:
    
        /** 
         * The drawable format defines the components used as an alternate
         * drawable for this cannel. If an alternate drawable is configured, the
         * channel uses the appropriate targets in place of the window's frame
         * buffer.
         * @version 1.0
         */
        enum Drawable
        {
            FB_WINDOW   = EQ_BIT_NONE, //!< Use the window's frame buffer
            FBO_COLOR   = EQ_BIT1,     //!< Use an FBO for color values
            FBO_DEPTH   = EQ_BIT2,     //!< Use an FBO for depth values
            FBO_STENCIL = EQ_BIT3      //!< Use an FBO for stencil values
        };
        
        /** Construct a new channel. @version 1.0 */
        EQ_EXPORT Channel( Window* parent );

        /** Destruct the channel. @version 1.0 */
        EQ_EXPORT virtual ~Channel();

        /**
         * @name Data Access
         */
        //@{
        /** @return the parent window. @version 1.0 */
        Window* getWindow() { return _window; }

        /** @return the parent window. @version 1.0 */
        const Window* getWindow() const { return _window; }

        /** @return the parent pipe. @version 1.0 */
        EQ_EXPORT Pipe* getPipe();

        /** @return the parent pipe. @version 1.0 */
        EQ_EXPORT const Pipe* getPipe() const;

        /** @return the parent node. @version 1.0 */
        EQ_EXPORT Node* getNode();

        /** @return the parent node. @version 1.0 */
        EQ_EXPORT const Node* getNode() const;

        /** @return the parent config. @version 1.0 */
        EQ_EXPORT Config* getConfig();

        /** @return the parent config. @version 1.0 */
        EQ_EXPORT const Config* getConfig() const;

        /** @return the parent server. @version 1.0 */
        EQ_EXPORT ServerPtr getServer();

        /** @return the name of the window. @version 1.0 */
        const std::string& getName() const { return _name; }

        /** 
         * Get the GLEW context for this channel.
         * 
         * The GLEW context is initialized during window initialization, and
         * provides access to OpenGL extensions. This function does not follow
         * the Equalizer naming conventions, since GLEW uses a function of this
         * name to automatically resolve OpenGL function entry
         * points. Therefore, any supported GL function can be called directly
         * from an initialized Channel.
         * 
         * @return the extended OpenGL function table for the channel's OpenGL
         *         context.
         * @version 1.0
         */
        EQ_EXPORT GLEWContext* glewGetContext();

        /** Const-version of glewGetContext() @version 1.0 */
        EQ_EXPORT const GLEWContext* glewGetContext() const;

        /** @return the window's object manager instance. @version 1.0 */
        EQ_EXPORT Window::ObjectManager* getObjectManager();

        /** @return the channel's drawable config. @version 1.0 */
        EQ_EXPORT const DrawableConfig& getDrawableConfig() const;

        /** 
         * Return the set of tasks this channel might execute in the worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @return the tasks.
         * @warning Experimental - may not be supported in the future
         */
        uint32_t getTasks() const { return _tasks; }

        /** 
         * Traverse this channel using a channel visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQ_EXPORT VisitorResult accept( ChannelVisitor& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQ_EXPORT VisitorResult accept( ChannelVisitor& visitor ) const;

        /** 
         * Set the near and far planes for this channel.
         * 
         * The given near and far planes update the current perspective and
         * orthographics frustum accordingly. Furthermore, they will be used in
         * the future by the server to compute the frusta.
         *
         * @param nearPlane the near plane.
         * @param farPlane the far plane.
         * @version 1.0
         */
        EQ_EXPORT void setNearFar( const float nearPlane, const float farPlane);

        /** Return a fixed unique color for this channel. @version 1.0 */
        const Vector3ub& getUniqueColor() const { return _color; }

        /**
         * Get the channel's native view.
         *
         * This function always returns the channel's native view, no matter in
         * which context it is called. Only destination channels have a native
         * view.
         *
         * @return the channel's native view, or 0 if it does not have one.
         * @sa getView()
         * @version 1.0
         */
        EQ_EXPORT View* getNativeView();

        /** const-version of getNativeView() @version 1.0 */
        EQ_EXPORT const View* getNativeView() const;

        /** @return the channel's native pixel viewport. @version 1.0 */
        const PixelViewport& getNativePixelViewPort() const
            { return _nativeContext.pvp; }

        /** @return the FBO used as an alternate frame buffer. @version 1.0*/
        EQ_EXPORT util::FrameBufferObject* getFrameBufferObject();

        /** Add a new statistics event for the current frame. @internal */
        EQ_EXPORT void addStatistic( Event& event );
        //@}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task. If they are called outside of a frame task method, they
         * return the channel's native parameter, e.g., a placeholder value for
         * the task decomposition parameters.
         */
        //@{
        /** @return the current draw buffer for glDrawBuffer. @version 1.0 */
        EQ_EXPORT uint32_t getDrawBuffer() const;

        /** @return the current read buffer for glReadBuffer. @version 1.0 */
        EQ_EXPORT uint32_t getReadBuffer() const;

        /** @return the current color mask for glColorMask. @version 1.0 */
        EQ_EXPORT const ColorMask& getDrawBufferMask() const;

        /**
         * @return the current pixel viewport for glViewport and glScissor.
         * @version 1.0
         */
        EQ_EXPORT const PixelViewport& getPixelViewport() const;

        /**
         * @return the current perspective frustum for glFrustum.
         * @version 1.0
         */
        EQ_EXPORT const Frustumf& getFrustum() const;

        /**
         * @return the current orthographic frustum for glOrtho.
         * @version 1.0
         */
        EQ_EXPORT const Frustumf& getOrtho() const;

        /**
         * @return the jitter vector for the current subpixel decomposition.
         * @version 1.0
         */
        EQ_EXPORT Vector2f getJitter() const;

        /**
         * Return the view matrix.
         *
         * The view matrix is part of the GL_MODEL*VIEW* matrix, and is
         * typically applied first to the GL_MODELVIEW matrix.
         * 
         * @return the head transformation matrix
         * @version 1.0
         */
        EQ_EXPORT const Matrix4f& getHeadTransform() const;

        /**
         * @return the fractional viewport wrt the destination view.
         * @version 1.0
         */
        EQ_EXPORT const Viewport& getViewport() const;

        /**
         * @return the database range for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT const Range& getRange() const;

        /**
         * @return the pixel decomposition for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT const Pixel& getPixel() const;

        /**
         * @return the subpixel decomposition for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT const SubPixel& getSubPixel() const;

        /**
         * @return the up/downscale zoom factor for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT const Zoom& getZoom() const;

        /**
         * @return the DPlex period for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT uint32_t getPeriod() const;

        /**
         * @return the DPlex phase for the current rendering task.
         * @version 1.0
         */
        EQ_EXPORT uint32_t getPhase() const;

        /**
         * Get the channel's current position wrt the destination channel.
         *
         * Note that computing this value from the current viewport and pixel
         * viewport is inaccurate because it neglects rounding errors of the
         * pixel viewport done by the server.
         *
         * @return the channel's current position wrt the destination channel.
         * @version 1.0
         */
        EQ_EXPORT const Vector2i& getPixelOffset() const;

        /** @return the currently rendered eye pass. @version 1.0 */
        EQ_EXPORT Eye getEye() const;

        /**
         * @return the list of input frames, used from frameAssemble().
         * @version 1.0
         */
        const FrameVector& getInputFrames() { return _inputFrames; }

        /**
         * @return the list of output frames, used from frameReadback().
         * @version 1.0
         */
        const FrameVector& getOutputFrames() { return _outputFrames; }

        /** 
         * Get the channel's current View.
         * 
         * During a frame task method, i.e., in one of the frameFoo functions,
         * the view is set to the view of the destination channel, that is, the
         * channel for which this channel is executing the rendering
         * task. Outside of a frame task method the native view of the channel,
         * or 0, is returned.
         * 
         * @return the channel's view, or 0 if it does not have a view.
         * @sa getNativeView()
         * @version 1.0
         */
        EQ_EXPORT View* getView();

        /** Const version of getView(). @version 1.0 */
        EQ_EXPORT const View* getView() const;

        /** 
         * Returns an orthographic frustum for 2D operations on the view.
         *
         * One unit of the frustum covers one pixel on screen. The frustum is
         * positioned relative to the view.
         *
         * @return the 2D orthographic frustum.
         * @version 1.0
         */
        EQ_EXPORT Frustumf getScreenFrustum() const;

        /** @internal  Undocumented - may not be supported in the future */
        EQ_EXPORT const Vector4i& getOverdraw() const;

        /** @internal  Undocumented - may not be supported in the future */
        EQ_EXPORT void setMaxSize( const Vector2i& size );

        /** @internal  Undocumented - may not be supported in the future */
        EQ_EXPORT uint32_t getTaskID() const;
        //@}

        /**
         * @name Operations
         *
         * Operations are only meaningful from within certain callbacks. They
         * are just convenience wrappers applying context-specific data to the
         * OpenGL state using the context-specific data access above.
         */
        //@{
        /** 
         * Apply the current rendering buffer, including the color mask.
         * @sa getReadBuffer() , getDrawBuffer(), getDrawBufferMask()
         * @version 1.0
         */
        EQ_EXPORT virtual void applyBuffer();

        /** 
         * Apply the current color mask.
         * @sa applyBuffer(), getDrawBufferMask()
         * @version 1.0
         */
        EQ_EXPORT virtual void applyColorMask() const;

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         * @sa getViewport()
         * @version 1.0
         */
        EQ_EXPORT virtual void applyViewport() const;

        /**
         * Apply the perspective frustum matrix for the current rendering task.
         *
         * If a sub-pixel decomposition is defined, the frustum is jittered by
         * the amount given by getJitter() to implement software
         * anti-aliasing. Applications which want to implement a different
         * multi-sampling algorithm, e.g., depth-of-field, have to re-implement
         * applyFrustum() accordingly.
         *
         * @sa getFrustum(), getJitter(), getSubPixel()
         * @version 1.0
         */
        EQ_EXPORT virtual void applyFrustum() const;

        /**
         * Apply the orthographic frustum matrix for the current rendering task.
         *
         * The same jitter as in applyFrustum() is applied.
         *
         * @sa getOrtho(), getJitter()
         * @version 1.0
         */
        EQ_EXPORT virtual void applyOrtho() const;

        /** 
         * Apply an orthographic frustum for pixel-based 2D operations. 
         *
         * One unit of the frustum covers one pixel on screen. The frustum is
         * positioned relative to the view.
         * @version 1.0
         */
        EQ_EXPORT void applyScreenFrustum() const;

        /** 
         * Apply the view transformation to position the view frustum.
         * @version 1.0
         */
        EQ_EXPORT virtual void applyHeadTransform() const;

        /** 
         * Apply the current alternate frame buffer.
         * @version 1.0
         */
        EQ_EXPORT virtual void applyFrameBufferObject();

        /** 
         * Rebind the current alternate FBO of the channel or window.
         * @version 1.0
         */
        EQ_EXPORT void bindFrameBuffer();        
        //@}

        /** 
         * Process a received event.
         *
         * The task of this method is to update the channel as necessary, and 
         * transform the event into an config event to be send to the 
         * application using Config::sendEvent().
         * 
         * @param event the received event.
         * @return true when the event was handled, false if not.
         * @version 1.0
         */
        EQ_EXPORT virtual bool processEvent( const Event& event );

        /** Draw a statistics overlay. @version 1.0 */
        EQ_EXPORT virtual void drawStatistics();

        /** Outline the current pixel viewport. @version 1.0 */
        EQ_EXPORT virtual void outlineViewport();

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in channel.cpp
        /** Integer attributes for a channel. @version 1.0 */
        enum IAttribute
        {
            /** Statistics gathering mode (OFF, FASTEST [ON], NICEST) */
            IATTR_HINT_STATISTICS,
            /** Use a send token for output frames (OFF, ON) */
            IATTR_HINT_SENDTOKEN,
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };
        
        /** @return the value of an integer attribute. @version 1.0 */
        EQ_EXPORT int32_t getIAttribute( const IAttribute attr ) const;
        /** @return the name of an integer attribute. @version 1.0 */
        EQ_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}

    protected:
        /** @internal */
        EQ_EXPORT void attachToSession( const uint32_t id, 
                                        const uint32_t instanceID, 
                                        net::Session* session );

        /** @name Actions */
        //@{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         * @version 1.0
         */
        void startFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         * @version 1.0
         */
        void releaseFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        void releaseFrameLocal( const uint32_t frameNumber ) { /* nop */ }

        /**
         * Setup the OpenGL state for a readback or assemble operation.
         *
         * The default implementation is very conservative and saves any state
         * which is potentially changed by the assembly routines. Applications
         * may overwrite this and resetAssemblyState() to optimize performance
         * in accordance with their rendering code.
         *
         * @version 1.0
         */
        EQ_EXPORT virtual void setupAssemblyState();

        /** Reset the OpenGL state after an assembly operation. @version 1.0 */
        EQ_EXPORT virtual void resetAssemblyState();
        //@}

        /**
         * @name Task Methods
         *
         * The task methods (callbacks) are called by Equalizer during rendering
         * to execute various rendering tasks. Each task method has a useful
         * default implementation, but at least frameDraw() is implemented by an
         * application.
         */
        //@{
        /** 
         * Initialize this channel.
         * 
         * @param initID the init identifier.
         * @version 1.0
         */
        EQ_EXPORT virtual bool configInit( const uint32_t initID );

        /** Exit this channel. @version 1.0 */
        EQ_EXPORT virtual bool configExit();

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of channel-specific data. This method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa Config::startFrame()
         * @version 1.0
         */
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber ) 
            { startFrame( frameNumber ); }

        /**
         * Finish rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates of
         * channel-specific data.  This method has to call releaseFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finish.
         * @version 1.0
         */
        virtual void frameFinish( const uint32_t frameID, 
                                  const uint32_t frameNumber ) 
            { releaseFrame( frameNumber ); }

        /** 
         * Finish drawing.
         * 
         * Called once per frame after the last draw operation. Typically
         * releases the local node thread synchronization for this frame.
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to finished with draw.
         * @version 1.0
         */
        virtual void frameDrawFinish( const uint32_t frameID, 
                                      const uint32_t frameNumber )
            { releaseFrameLocal( frameNumber ); }

        /** 
         * Clear the frame buffer.
         *
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        EQ_EXPORT virtual void frameClear( const uint32_t frameID );

        /** 
         * Draw the scene.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        EQ_EXPORT virtual void frameDraw( const uint32_t frameID );

        /** 
         * Assemble all input frames.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @sa getInputFrames()
         * @version 1.0
         */
        EQ_EXPORT virtual void frameAssemble( const uint32_t frameID );

        /** 
         * Read back the rendered frame buffer into all output frames.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @sa getOutputFrames()
         * @version 1.0
         */
        EQ_EXPORT virtual void frameReadback( const uint32_t frameID );

        /** 
         * Start updating a destination channel.
         *
         * Called once on each destination channel, e.g., channels which are
         * defined by a view/segment intersection, after frameStart to update a
         * part of a display.
         * 
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        virtual void frameViewStart( const uint32_t frameID ) { /* nop */ }

        /** 
         * Finish updating a destination channel.
         *
         * Called once on each destination channel, e.g., channels which are
         * defined by a view/segment intersection, before frameFinish to update
         * a part of a view.
         * 
         * This is typically used to do operations on the output channel after
         * it has been fully updated, e.g., to draw a 2D overlay.
         *
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        virtual void frameViewFinish( const uint32_t frameID ) { /* nop */ }
        //@}

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         * @version 1.0
         */
        EQ_EXPORT void setErrorMessage( const std::string& message );
        //@}

    private:
        //-------------------- Members --------------------
        /** The parent window. */
        Window* const _window;
        friend class Window;

        /** The name. */
        std::string    _name;

        /** The native render context parameters of this channel. */
        RenderContext _nativeContext;

        /** The rendering parameters for the current operation. */
        RenderContext* _context;

        /** The channel's drawable config (FBO). */
        DrawableConfig _drawableConfig;

        /** A unique color assigned by the server during config init. */
        Vector3ub _color;

        /** The reason for the last error. */
        std::string     _error;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING
        };

        /** The configInit/configExit state. */
        State _state;

        /** server-supplied vector of output frames for current task. */
        FrameVector _outputFrames;

        /** server-supplied vector of input frames for current task. */
        FrameVector _inputFrames;

        /** true if the pvp is immutable, false if the vp is immutable */
        bool _fixedPVP;

        /** Used as an alternate drawable. */       
        util::FrameBufferObject* _fbo; 
        
        /** Alternate drawable definition. */
        uint32_t _drawable;
        
        /** The statistics events gathered during the current frame. */
        std::vector< Statistic > _statistics;

        /** The initial channel size, used for view resize events. */
        Vector2i _initialSize;

        /** The maximum size (used to clamp overdraw right now) */
        Vector2i _maxSize;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        //-------------------- Methods --------------------
        /** 
         * Set the channel's fractional viewport wrt its parent pipe.
         *
         * Updates the pixel viewport accordingly.
         * 
         * @param vp the fractional viewport.
         */
        void _setViewport( const Viewport& vp );

        /** 
         * Set the channel's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport accordingly.
         * 
         * @param pvp the viewport in pixels.
         */
        void _setPixelViewport( const PixelViewport& pvp );

        /** Notification of window pvp change. */
        void _notifyViewportChanged();

        /** Setup the current rendering context. */
        void _setRenderContext( RenderContext& context );

        /** Initialize the FBO */
        bool _configInitFBO();

        /** Initialize the channel's drawable config. */
        void _initDrawableConfig();

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* The command handler functions. */
        net::CommandResult _cmdConfigInit( net::Command& command );
        net::CommandResult _cmdConfigExit( net::Command& command );
        net::CommandResult _cmdFrameStart( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
        net::CommandResult _cmdFrameClear( net::Command& command );
        net::CommandResult _cmdFrameDraw( net::Command& command );
        net::CommandResult _cmdFrameDrawFinish( net::Command& command );
        net::CommandResult _cmdFrameAssemble( net::Command& command );
        net::CommandResult _cmdFrameReadback( net::Command& command );
        net::CommandResult _cmdFrameTransmit( net::Command& command );
        net::CommandResult _cmdFrameViewStart( net::Command& command );
        net::CommandResult _cmdFrameViewFinish( net::Command& command );
    };
}

#endif // EQ_CHANNEL_H

