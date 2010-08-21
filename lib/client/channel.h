
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

#include <eq/client/event.h>          // member
#include <eq/client/eye.h>            // enum
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum
#include <eq/client/window.h>         // nested Window::ObjectManager class
#include <eq/client/os.h>             // GLEWContext

#include <eq/fabric/channel.h>        // base class

namespace eq
{
namespace util
{
    class FrameBufferObject;
}
    /**
     * A channel represents a two-dimensional viewport within a Window.
     *
     * The channel is the basic rendering entity. It represents a 2D rendering
     * area within a Window. It executes all rendering-relevant tasks, such as
     * clear, draw, assemble and readback. Each rendering task is using its own
     * RenderContext, which is computed by the server based on the rendering
     * description of the current configuration.
     *
     * @sa fabric::Channel
     */
    class Channel : public fabric::Channel< Window, Channel >
    {
    public:
        /** Construct a new channel. @version 1.0 */
        EQ_EXPORT Channel( Window* parent );

        /** Destruct the channel. @version 1.0 */
        EQ_EXPORT virtual ~Channel();

        /** @name Data Access */
        //@{
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
        EQ_EXPORT const GLEWContext* glewGetContext() const;

        /** @return the window's object manager instance. @version 1.0 */
        EQ_EXPORT Window::ObjectManager* getObjectManager();

        /** @return the channel's drawable config. @version 1.0 */
        EQ_EXPORT const DrawableConfig& getDrawableConfig() const;

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
        /**
         * @return the jitter vector for the current subpixel decomposition.
         * @version 1.0
         */
        EQ_EXPORT Vector2f getJitter() const;

        /**
         * @return the list of input frames, used from frameAssemble().
         * @version 1.0
         */
        const Frames& getInputFrames() { return _inputFrames; }

        /**
         * @return the list of output frames, used from frameReadback().
         * @version 1.0
         */
        const Frames& getOutputFrames() { return _outputFrames; }

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
        EQ_EXPORT void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         * @version 1.0
         */
        EQ_EXPORT void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        EQ_EXPORT void releaseFrameLocal( const uint32_t frameNumber );

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
        EQ_EXPORT virtual void frameStart( const uint32_t frameID,
                                           const uint32_t frameNumber );

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
        EQ_EXPORT virtual void frameFinish( const uint32_t frameID, 
                                            const uint32_t frameNumber );

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
        EQ_EXPORT virtual void frameDrawFinish( const uint32_t frameID, 
                                                const uint32_t frameNumber );

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
        EQ_EXPORT virtual void frameViewStart( const uint32_t frameID );

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
        EQ_EXPORT virtual void frameViewFinish( const uint32_t frameID );
        //@}

        /** Notification that parameters influencing the vp/pvp have changed.*/
        EQ_EXPORT virtual void notifyViewportChanged();

    private:
        //-------------------- Members --------------------
        friend class fabric::Window< Pipe, Window, Channel >;

        /** The channel's drawable config (FBO). */
        DrawableConfig _drawableConfig;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING
        };

        /** The configInit/configExit state. */
        State _state;

        /** server-supplied vector of output frames for current task. */
        Frames _outputFrames;

        /** server-supplied vector of input frames for current task. */
        Frames _inputFrames;

        /** Used as an alternate drawable. */       
        util::FrameBufferObject* _fbo; 
        
        /** The statistics events gathered during the current frame. */
        std::vector< Statistic > _statistics;

        /** The initial channel size, used for view resize events. */
        Vector2i _initialSize;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        //-------------------- Methods --------------------
        /** Setup the current rendering context. */
        void _setRenderContext( RenderContext& context );

        /** Initialize the FBO */
        bool _configInitFBO();

        /** Initialize the channel's drawable config. */
        void _initDrawableConfig();

        /* The command handler functions. */
        bool _cmdConfigInit( net::Command& command );
        bool _cmdConfigExit( net::Command& command );
        bool _cmdFrameStart( net::Command& command );
        bool _cmdFrameFinish( net::Command& command );
        bool _cmdFrameClear( net::Command& command );
        bool _cmdFrameDraw( net::Command& command );
        bool _cmdFrameDrawFinish( net::Command& command );
        bool _cmdFrameAssemble( net::Command& command );
        bool _cmdFrameReadback( net::Command& command );
        bool _cmdFrameTransmit( net::Command& command );
        bool _cmdFrameViewStart( net::Command& command );
        bool _cmdFrameViewFinish( net::Command& command );
    };
}

#endif // EQ_CHANNEL_H

