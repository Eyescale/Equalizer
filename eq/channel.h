
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include <eq/api.h>
#include <eq/types.h>

#include <eq/fabric/channel.h>        // base class

namespace eq
{
namespace detail { class Channel; struct RBStat; }

/**
 * A channel represents a two-dimensional viewport within a Window.
 *
 * The channel is the basic rendering entity. It represents a 2D rendering area
 * within a Window. It executes all rendering-relevant tasks, such as clear,
 * draw, assemble and readback. Each rendering task is using its own
 * RenderContext, which is computed by the server based on the rendering
 * description of the current configuration.
 *
 * @sa fabric::Channel
 */
class Channel : public fabric::Channel< Window, Channel >
{
public:
    /** Construct a new channel. @version 1.0 */
    EQ_API explicit Channel( Window* parent );

    /** Destruct the channel. @version 1.0 */
    EQ_API virtual ~Channel();

    /** @name Data Access */
    //@{
    EQ_API co::CommandQueue* getPipeThreadQueue(); //!< @internal
    EQ_API co::CommandQueue* getCommandThreadQueue(); //!< @internal
    EQ_API uint32_t getCurrentFrame() const; //!< @internal render thr only

    /** @internal */
    bool waitFrameFinished( uint32_t frame, uint32_t timeout ) const;

    /**
     * @return true if this channel is stopped, false otherwise.
     * @version 1.0
     */
    EQ_API bool isStopped() const;

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
     * provides access to OpenGL extensions. This function does not follow the
     * Equalizer naming conventions, since GLEW uses a function of this name to
     * automatically resolve OpenGL function entry points. Therefore, any
     * supported GL function can be called directly from an initialized Channel.
     *
     * @return the extended OpenGL function table for the channel's OpenGL
     *         context.
     * @version 1.0
     */
    EQ_API const GLEWContext* glewGetContext() const;

    /** @return the window's object manager instance. @version 1.0 */
    EQ_API util::ObjectManager& getObjectManager();

    /** @return the channel's drawable config. @version 1.0 */
    EQ_API const DrawableConfig& getDrawableConfig() const;

    /**
     * Get the channel's native view.
     *
     * This function always returns the channel's native view, no matter in
     * which context it is called. Only destination channels have a native view.
     *
     * @return the channel's native view, or 0 if it does not have one.
     * @sa getView()
     * @version 1.0
     */
    EQ_API View* getNativeView();

    /** const-version of getNativeView() @version 1.0 */
    EQ_API const View* getNativeView() const;

    /** @return a fixed unique color for this channel. @version 1.0 */
    EQ_API const Vector3ub& getUniqueColor() const;

    /** @internal Add a new statistics event for the current frame. */
    EQ_API void addStatistic( Event& event );
    //@}

    /**
     * @name Context-specific data access.
     *
     * The data returned by these methods depends on the context (callback) they
     * are called from, typically the data for the current rendering task. If
     * they are called outside of a frame task method, they return the channel's
     * native parameter or a placeholder value for the task decomposition
     * parameters.
     */
    //@{
    /**
     * @return the jitter vector for the current subpixel decomposition.
     * @version 1.0
     */
    EQ_API virtual Vector2f getJitter() const;

    /**
     * Get the channel's current View.
     *
     * During a frame task method, i.e., in one of the frameFoo functions, the
     * view is set to the view of the destination channel, that is, the channel
     * for which this channel is executing the rendering task. Outside of a
     * frame task method the native view of the channel, or 0, is returned.
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
     * Operations are only meaningful from within certain callbacks. They are
     * just convenience wrappers applying context-specific data to the OpenGL
     * state using the context-specific data access above.
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
     * Apply the frustum matrix for the current rendering task.
     *
     * If a sub-pixel decomposition is defined, the frustum is jittered by the
     * amount given by getJitter() to implement software
     * anti-aliasing. Applications which want to implement a different
     * multi-sampling algorithm, e.g., depth-of-field, have to re-implement
     * getJitter() or applyFrustum() accordingly.
     *
     * @sa useOrtho(), getJitter(), getSubPixel()
     * @version 1.0
     */
    EQ_API virtual void applyFrustum() const;

    /**
     * Apply the perspective frustum matrix for the current rendering task.
     * @version 1.0
     */
    EQ_API virtual void applyPerspective() const;

    /**
     * Apply the orthographic frustum matrix for the current rendering task.
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
     * Apply the transformation to position the view frustum.
     * @version 1.0
     * @sa useOrtho()
     */
    EQ_API virtual void applyHeadTransform() const;

    /**
     * Apply the transformation to position the perspective view frustum.
     * @version 1.0
     */
    EQ_API virtual void applyPerspectiveTransform() const;

    /**
     * Apply the transformation to position the orthographic view frustum.
     * @version 1.0
     */
    EQ_API virtual void applyOrthoTransform() const;

    /**
     * Apply the state for pixel-based 2D overlay rendering operations.
     * @version 1.11
     */
    EQ_API void applyOverlayState();

    /**
     * Reset the overlay state setup by applyOverlayState()
     * @version 1.11
     */
    EQ_API void resetOverlayState();

    /**
     * Rebind the window frame buffer.
     * @version 1.0
     */
    EQ_API void bindFrameBuffer();

    /**
     * Rebind the window draw buffer.
     * @version 1.9
     */
    EQ_API void bindDrawFrameBuffer();
    //@}

    /** @name Region of Interest. */
    //@{
    /**
     * Reset the declared regions of interest.
     *
     * Called from frameStart and frameClear to reset the area to be used to
     * optimize compositing and load balancing for each frame.
     * @version 1.3
     */
    EQ_API virtual void resetRegions();

    /**
     * Declare a region covered by the current draw or assemble operation.
     *
     * The region is relative to the current pixel viewport. It is clipped
     * against the current pixel viewport of the channel. Called with the full
     * pixel viewport after frameDraw if no region has been declared.
     *
     * Declaring a single, empty region causes this channel to not read back any
     * pixel data, i.e., if it did not draw anything.
     *
     * The implementation might merge or split the declared regions.
     *
     * @version 1.3
     */
    EQ_API virtual void declareRegion( const eq::PixelViewport& region );

    /**
     * Convenience method to declare a region in relative coordinates.
     *
     * The given viewport is relative to the current pixel viewport.
     * @version 1.3
     */
    EQ_API void declareRegion( const eq::Viewport& vp );

    /** @return a region covering all declared regions. @version 1.3 */
    EQ_API PixelViewport getRegion() const;

    /**
     * Get the current regions of interest.
     *
     * The returned regions are guaranteed not to overlap with each
     * other. Therefore they may differ in number and size from the declared
     * regions. The actual algorithm to create the non-overlapping regions is
     * unspecified and may change in the future.
     *
     * @return current regions of interest.
     * @version 1.3
     */
    EQ_API const PixelViewports& getRegions() const;
    //@}

    /** @name Events */
    //@{
    /**
     * Send a channel error event to the application node.
     *
     * @param error the error code.
     * @version 1.7.1
     */
    EQ_API EventOCommand sendError( const uint32_t error );

    /**
     * Process a received event.
     *
     * The task of this method is to update the channel as necessary, and
     * transform the event into a config event to be send to the application
     * using Config::sendEvent().
     *
     * @param event the received event.
     * @return true when the event was handled, false if not.
     * @version 1.0
     */
    EQ_API virtual bool processEvent( const Event& event );
    //@}

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

    /**
     * Add a listener that gets notified everytime a new frame was rendered for
     * a destination channel.
     *
     * The channel does not take ownership of the listener. The notification
     * always happens in the render thread.
     *
     * @param listener the new listener to add
     * @version 1.9
     */
    EQ_API void addResultImageListener( ResultImageListener* listener );

    /**
     * Remove a result image listener to stop receival of notifications on new
     * images.
     *
     * @param listener the new listener to remove
     * @version 1.9
     */
    EQ_API void removeResultImageListener( ResultImageListener* listener );

    /**
     * @return filename for image if SATTR_DUMP_IMAGE,
     *         default "getCurrentFrame().rgb"
     * @version 1.9
     */
    EQ_API virtual std::string getDumpImageFileName() const;

protected:
    /** @internal */
    EQ_API void attach( const uint128_t& id, const uint32_t instanceID );

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
     * @param frames the input frames.
     * @version 1.7.3
     */
    EQ_API virtual void frameAssemble( const uint128_t& frameID,
                                       const Frames& frames );

    /**
     * Read back the rendered frame buffer into the output frames.
     *
     * Called 0 to n times during one frame.
     *
     * @param frameID the per-frame identifier.
     * @param frames the output frames.
     * @version 1.7.3
     * @version 1.0
     */
    EQ_API virtual void frameReadback( const uint128_t& frameID,
                                       const Frames& frames );

    /**
     * Start updating a destination channel.
     *
     * Called once for each destination channel's eye pass, after frameStart()
     * to update a part of a display. Destination channels are the final display
     * channels, e.g., channels which are defined by a view/segment
     * intersection.
     *
     * @param frameID the per-frame identifier.
     * @version 1.0
     */
    EQ_API virtual void frameViewStart( const uint128_t& frameID );

    /**
     * Finish updating a destination channel.
     *
     * Called once for each destination channel's eye pass, before frameFinish()
     * to update a part of a display. Destination channels are the final display
     * channels, e.g., channels which are defined by a view/segment
     * intersection.
     *
     * This is typically used to do operations on the output channel after
     * it has been fully updated, e.g., to draw a 2D overlay or to perform
     * post-processing on the rendered image.
     *
     * @param frameID the per-frame identifier.
     * @version 1.0
     */
    EQ_API virtual void frameViewFinish( const uint128_t& frameID );

    /**
     * Draw 2D overlay content on a destination channel.
     *
     * This is called by frameViewFinish().
     *
     * @param frameID the per-frame identifier.
     * @version 1.11
     */
    EQ_API virtual void frameDrawOverlay( const uint128_t& frameID );
    //@}

    /** Start a batch of tile rendering operations. @version 1.1.6 */
    virtual void frameTilesStart( const uint128_t& /*frameID*/ ) {}

    /** Finish a batch of tile rendering operations. @version 1.1.6 */
    virtual void frameTilesFinish( const uint128_t& /*frameID*/ ) {}

    /** Notification that parameters influencing the vp/pvp have changed.*/
    EQ_API virtual void notifyViewportChanged();

    /**
     * Notify interruption of the rendering.
     *
     * This method is called from the Client command thread, as opposed to the
     * rendering thread. Its purpose is to cause the rendering thread to stop
     * its operations as soon as possible. Normal rendering shall recommence
     * after the given frame.
     *
     * @param lastFrameNumber stop rendering until this frame has been
     *        processed.
     * @version 1.0
     */
    EQ_API virtual void notifyStopFrame( const uint32_t lastFrameNumber );

private:
    detail::Channel* const _impl;
    friend class fabric::Window< Pipe, Window, Channel, WindowSettings >;

    //-------------------- Methods --------------------
    /** Setup the current rendering context. */
    void _overrideContext( RenderContext& context );

    /** Initialize the channel's drawable config. */
    void _initDrawableConfig();

    /** Tile render loop. */
    void _frameTiles( RenderContext& context, const bool isLocal,
                      const uint128_t& queueID, const uint32_t tasks,
                      const co::ObjectVersions& frames );

    /** Reference the frame for an async operation. */
    void _refFrame( const uint32_t frameNumber );

    /** Check for and send frame finish reply. */
    void _unrefFrame( const uint32_t frameNumber );

    /** Transmit one image of a frame to one node. */
    void _transmitImage( const co::ObjectVersion& frameDataVersion,
                         const uint128_t& nodeID,
                         const co::NodeID& netNodeID,
                         const uint64_t imageIndex,
                         const uint32_t frameNumber,
                         const uint32_t taskID );

    void _frameReadback( const uint128_t& frameID,
                         const co::ObjectVersions& frames );
    void _finishReadback( const co::ObjectVersion& frameDataVersion,
                          const uint64_t imageIndex,
                          const uint32_t frameNumber,
                          const uint32_t taskID,
                          const std::vector< uint128_t >& nodes,
                          const co::NodeIDs& netNodes );

    bool _asyncFinishReadback( const std::vector< size_t >& imagePos,
                               const Frames& frames );

    void _asyncTransmit( FrameDataPtr frame, const uint32_t frameNumber,
                         const uint64_t image,
                         const std::vector< uint128_t >& nodes,
                         const co::NodeIDs& netNodes,
                         const uint32_t taskID );

    void _setReady( const bool async, detail::RBStat* stat,
                    const Frames& frames );
    void _asyncSetReady( const FrameDataPtr frame, detail::RBStat* stat,
                         const std::vector< uint128_t >& nodes,
                         const co::NodeIDs& netNodes );

    void _setReady( FrameDataPtr frame, detail::RBStat* stat,
                    const std::vector< uint128_t >& nodes,
                    const co::NodeIDs& netNodes );

    /** Getsthe channel's current input queue. */
    co::QueueSlave* _getQueue( const uint128_t& queueID );

    Frames _getFrames( const co::ObjectVersions& frameIDs,
                       const bool isOutput );

    void _createTransferWindow();
    void _deleteTransferWindow();

    /* The command handler functions. */
    bool _cmdConfigInit( co::ICommand& command );
    bool _cmdConfigExit( co::ICommand& command );
    bool _cmdFrameStart( co::ICommand& command );
    bool _cmdFrameFinish( co::ICommand& command );
    bool _cmdFrameClear( co::ICommand& command );
    bool _cmdFrameDraw( co::ICommand& command );
    bool _cmdFrameDrawFinish( co::ICommand& command );
    bool _cmdFrameAssemble( co::ICommand& command );
    bool _cmdFrameReadback( co::ICommand& command );
    bool _cmdFinishReadback( co::ICommand& command );
    bool _cmdFrameSetReady( co::ICommand& command );
    bool _cmdFrameTransmitImage( co::ICommand& command );
    bool _cmdFrameSetReadyNode( co::ICommand& command );
    bool _cmdFrameViewStart( co::ICommand& command );
    bool _cmdFrameViewFinish( co::ICommand& command );
    bool _cmdStopFrame( co::ICommand& command );
    bool _cmdFrameTiles( co::ICommand& command );
    bool _cmdDeleteTransferWindow( co::ICommand& command );

    LB_TS_VAR( _pipeThread );
};
}

#endif // EQ_CHANNEL_H
