
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include <eq/event.h>          // member
#include <eq/eye.h>            // enum
#include <eq/types.h>
#include <eq/visitorResult.h>  // enum
#include <eq/window.h>         // nested Window::ObjectManager class
#include <eq/os.h>             // GLEWContext

#include <eq/fabric/channel.h>        // base class

namespace eq
{
    struct ChannelFrameTransmitPacket;

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
        EQ_API Channel( Window* parent );

        /** Destruct the channel. @version 1.0 */
        EQ_API virtual ~Channel();

        /** @name Data Access */
        //@{
        EQ_API co::CommandQueue* getPipeThreadQueue(); //!< @internal
        EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal

        /**
         * @return true if this channel is stopped, false otherwise.
         * @version 1.0 
         */
        bool isStopped() const { return (_state == STATE_STOPPED); }

        /** @return the parent pipe. @version 1.0 */
        EQ_API Pipe* getPipe();

        /** @return the parent pipe. @version 1.0 */
        EQ_API const Pipe* getPipe() const;

        /** @return the parent node. @version 1.0 */
        EQ_API Node* getNode();

        /** @return the parent node. @version 1.0 */
        EQ_API const Node* getNode() const;

        /** @return the parent config. @version 1.0 */
        EQ_API Config* getConfig();

        /** @return the parent config. @version 1.0 */
        EQ_API const Config* getConfig() const;

        /** @return the parent server. @version 1.0 */
        EQ_API ServerPtr getServer();

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
        EQ_API const GLEWContext* glewGetContext() const;

        /** @return the window's object manager instance. @version 1.0 */
        EQ_API Window::ObjectManager* getObjectManager();

        /** @return the channel's drawable config. @version 1.0 */
        EQ_API const DrawableConfig& getDrawableConfig() const;

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
        EQ_API View* getNativeView();

        /** const-version of getNativeView() @version 1.0 */
        EQ_API const View* getNativeView() const;

        /** @return the FBO used as an alternate frame buffer. @version 1.0*/
        EQ_API util::FrameBufferObject* getFrameBufferObject();

        /** @internal Add a new statistics event for the current frame. */
        EQ_API void addStatistic( Event& event, const uint32_t index );
        //@}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task. If they are called outside of a frame task method, they
         * return the channel's native parameter or a placeholder value for
         * the task decomposition parameters.
         */
        //@{
        /**
         * @return the jitter vector for the current subpixel decomposition.
         * @version 1.0
         */
        EQ_API Vector2f getJitter() const;

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
        EQ_API View* getView();

        /** Const version of getView(). @version 1.0 */
        EQ_API const View* getView() const;

        /** 
         * Returns an orthographic frustum for 2D operations on the view.
         *
         * One unit of the frustum covers one pixel on screen. The frustum is
         * positioned relative to the view.
         *
         * @return the 2D orthographic frustum.
         * @version 1.0
         */
        EQ_API Frustumf getScreenFrustum() const;
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
        EQ_API virtual void applyBuffer();

        /** 
         * Apply the current color mask.
         * @sa applyBuffer(), getDrawBufferMask()
         * @version 1.0
         */
        EQ_API virtual void applyColorMask() const;

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         * @sa getViewport()
         * @version 1.0
         */
        EQ_API virtual void applyViewport() const;

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
        EQ_API virtual void applyFrustum() const;

        /**
         * Apply the orthographic frustum matrix for the current rendering task.
         *
         * The same jitter as in applyFrustum() is applied.
         *
         * @sa getOrtho(), getJitter()
         * @version 1.0
         */
        EQ_API virtual void applyOrtho() const;

        /** 
         * Apply an orthographic frustum for pixel-based 2D operations. 
         *
         * One unit of the frustum covers one pixel on screen. The frustum is
         * positioned relative to the view.
         * @version 1.0
         */
        EQ_API void applyScreenFrustum() const;

        /** 
         * Apply the view transformation to position the view frustum.
         * @version 1.0
         */
        EQ_API virtual void applyHeadTransform() const;

        /** 
         * Apply the current alternate frame buffer.
         * @version 1.0
         */
        EQ_API virtual void applyFrameBufferObject();

        /** 
         * Rebind the current alternate FBO of the channel or window.
         * @version 1.0
         */
        EQ_API void bindFrameBuffer();        
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
        EQ_API virtual bool processEvent( const Event& event );

        /** Draw a statistics overlay. @version 1.0 */
        EQ_API virtual void drawStatistics();

        /** Outline the current pixel viewport. @version 1.0 */
        EQ_API virtual void outlineViewport();

        /**
          * @internal
          * Change the latency.
          *
          * @param latency the new latency.
          */
        void changeLatency( const uint32_t latency );

        /** @return a fixed unique color for this channel. @version 1.0 */
        const Vector3ub& getUniqueColor() const { return _color; }

    protected:
        /** @internal */
        EQ_API void attach( const co::base::UUID& id, const uint32_t instanceID );

        /** @name Actions */
        //@{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         * @version 1.0
         */
        EQ_API void startFrame( const uint32_t frameNumber );

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         * @version 1.0
         */
        EQ_API void releaseFrame( const uint32_t frameNumber );

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         * @version 1.0
         */
        EQ_API void releaseFrameLocal( const uint32_t frameNumber );

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
        EQ_API virtual void setupAssemblyState();

        /** Reset the OpenGL state after an assembly operation. @version 1.0 */
        EQ_API virtual void resetAssemblyState();
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
        EQ_API virtual bool configInit( const uint128_t& initID );

        /** Exit this channel. @version 1.0 */
        EQ_API virtual bool configExit();

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
        EQ_API virtual void frameStart( const uint128_t& frameID,
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
        EQ_API virtual void frameFinish( const uint128_t& frameID, 
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
        EQ_API virtual void frameDrawFinish( const uint128_t& frameID, 
                                             const uint32_t frameNumber );

        /** 
         * Clear the frame buffer.
         *
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        EQ_API virtual void frameClear( const uint128_t& frameID );

        /** 
         * Draw the scene.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @version 1.0
         */
        EQ_API virtual void frameDraw( const uint128_t& frameID );

        /** 
         * Assemble all input frames.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @sa getInputFrames()
         * @version 1.0
         */
        EQ_API virtual void frameAssemble( const uint128_t& frameID );

        /** 
         * Read back the rendered frame buffer into all output frames.
         * 
         * Called 0 to n times during one frame.
         * 
         * @param frameID the per-frame identifier.
         * @sa getOutputFrames()
         * @version 1.0
         */
         EQ_API virtual void frameReadback( const uint128_t& frameID );

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
        EQ_API virtual void frameViewStart( const uint128_t& frameID );

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
        EQ_API virtual void frameViewFinish( const uint128_t& frameID );
        //@}

        /** Notification that parameters influencing the vp/pvp have changed.*/
        EQ_API virtual void notifyViewportChanged();

        /** 
         * Notify interruption of the rendering.
         *
         * This method is called from the Client command thread, as opposed to
         * the rendering thread. Its purpose is to cause the rendering thread
         * to stop its operations as soon as possible. Normal rendering shall
         * recommence after the given frame.
         *
         * @param lastFrameNumber stop rendering until this frame has been
         *        processed.
         * @version 1.0
         */
        virtual void notifyStopFrame( const uint32_t lastFrameNumber ) {}

    private:
        //-------------------- Members --------------------
        friend class fabric::Window< Pipe, Window, Channel >;

        /** The channel's drawable config (FBO). */
        DrawableConfig _drawableConfig;

        enum State
        {
            STATE_STOPPED,
            STATE_INITIALIZING,
            STATE_RUNNING,
            STATE_FAILED
        };

        /** The configInit/configExit state. */
        State _state;

        /** server-supplied vector of output frames for current task. */
        Frames _outputFrames;

        /** server-supplied vector of input frames for current task. */
        Frames _inputFrames;

        /** Used as an alternate drawable. */
        util::FrameBufferObject* _fbo; 

        /** A random, unique color for this channel. */
        Vector3ub _color;

        typedef std::vector< Statistic > Statistics;
        struct FrameStatistics
        {
            Statistics data; //!< all events for one frame
            co::base::a_int32_t used; //!< reference count by pipe and xmit thread
        };

        typedef std::vector< FrameStatistics > StatisticsRB;

        /** The statistics events gathered during the current frame. */
        StatisticsRB _statistics;
        
        /* Index of current vector StatisticsRB */ 
        uint32_t _statisticsIndex;
        friend class ChannelStatistics;

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

        /** Check for and send frame finish reply. */
        void _unrefFrame( const uint32_t frameNumber, const uint32_t index );

        /** Transmit the frame data to the nodeID. */
        void _transmit( const ChannelFrameTransmitPacket* packet );

        /* The command handler functions. */
        bool _cmdConfigInit( co::Command& command );
        bool _cmdConfigExit( co::Command& command );
        bool _cmdFrameStart( co::Command& command );
        bool _cmdFrameFinish( co::Command& command );
        bool _cmdFrameClear( co::Command& command );
        bool _cmdFrameDraw( co::Command& command );
        bool _cmdFrameDrawFinish( co::Command& command );
        bool _cmdFrameAssemble( co::Command& command );
        bool _cmdFrameReadback( co::Command& command );
        bool _cmdFrameTransmit( co::Command& command );
        bool _cmdFrameTransmitAsync( co::Command& command );
        bool _cmdFrameViewStart( co::Command& command );
        bool _cmdFrameViewFinish( co::Command& command );
        bool _cmdStopFrame( co::Command& command );
    };
}

#endif // EQ_CHANNEL_H

