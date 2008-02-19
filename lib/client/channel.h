/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include <eq/client/colorMask.h>     // enum used
#include <eq/client/eye.h>           // enum used
#include <eq/client/frame.h>         // used in inline method
#include <eq/client/pixelViewport.h> // member
#include <eq/client/window.h>        // used in inline method

#include <vmmlib/vmmlib.h>           // Frustum definition 

namespace eq
{
    class Channel;
    class Pixel;
    class Node;
    class Range;
    class SceneObject;
    struct RenderContext;

    /**
     * A channel represents a two-dimensional viewport within a Window.
     *
     * The channel is the basic rendering entity. It executes all
     * rendering-relevant tasks, such as clear, draw, assemble and readback. It
     * is a child of a Window.
     */
    class EQ_EXPORT Channel : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new channel.
         */
        Channel( Window* parent );

        /**
         * @name Data Access
         */
        //*{
        Window* getWindow()   const
            { return _window; }
        Pipe*   getPipe()   const
            { return ( _window ? _window->getPipe() : 0 );}
        Node* getNode() const 
            { return ( _window ? _window->getNode() : 0 );}
        Config* getConfig() const 
            { return ( _window ? _window->getConfig() : 0 );}
        eqBase::RefPtr< Server > getServer() const 
            { return ( _window ? _window->getServer() : 0 );}

        GLEWContext* glewGetContext() { return _window->glewGetContext(); }

        const std::string& getName() const { return _name; }

        /** 
         * Set the near and far planes for this channel.
         * 
         * The near and far planes are set during initialisation and are
         * inherited by source channels contributing to the rendering of this
         * channel. Dynamic near and far planes can be applied using
         * applyNearFar.
         *
         * @param nearPlane the near plane.
         * @param farPlane the far plane.
         */
        void setNearFar( const float nearPlane, const float farPlane );

        /** 
         * Returns the current near and far planes for this channel.
         *
         * The current near and far plane depends on the context from which this
         * function is called.
         * 
         * @param nearPlane a pointer to store the near plane.
         * @param farPlane a pointer to store the far plane.
         */
        void getNearFar( float *nearPlane, float *farPlane );

        /** Return a stable, unique color for this channel. */
        const vmml::Vector3ub& getUniqueColor() const { return _color; }
        //*}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task.
         */
        //*{
        /** @return the channel's current draw buffer. */
        uint32_t getDrawBuffer() const;

        /** @return the channel's current read buffer. */
        uint32_t getReadBuffer() const;

        /** @return the channel's current color mask for drawing. */
        const ColorMask& getDrawBufferMask() const;

        /** @return the channel's current pixel viewport. */
        const PixelViewport& getPixelViewport() const;

        /**
         * Get the channel's current position wrt the destination channel.
         *
         * Note that computing this value from the current viewport and pixel
         * viewport inaccurate because it neglects rounding of the pixel
         * viewport done by the server.
         *
         * @return the channel's current position wrt the destination channel.
         */
        const vmml::Vector2i& getPixelOffset() const;

        /** @return the view frustum for the current rendering task. */
        const vmml::Frustumf& getFrustum() const;

        /** @return the fractional viewport wrt the destination. */
        const Viewport& getViewport() const;

        /** @return the database range for the current rendering task. */
        const Range& getRange() const;

        /** @return the pixel decomposition for the current rendering task. */
        const Pixel& getPixel() const;

        /** @return the currently rendered eye pass. */
        Eye getEye() const;

        /**
         * @return the modelling transformation to position and orient the view
         *         frustum.
         */
        const vmml::Matrix4f& getHeadTransform() const;

        /** @return the list of input frames, used from frameAssemble(). */
        const std::vector<Frame*>& getInputFrames() { return _inputFrames; }

        /** @return the list of output frames, used from frameReadback(). */
        const std::vector<Frame*>& getOutputFrames() { return _outputFrames; }
        //*}

        /**
         * @name Operations
         *
         * Operations are only meaningfull from within certain callbacks. They
         * are just convenience wrappers applying context-specific data to the
         * OpenGL state.
         */
        //*{
        /** 
         * Apply the current rendering buffer, including the color mask.
         */
        virtual void applyBuffer() const;

        /** 
         * Apply the current color mask.
         */
        virtual void applyColorMask() const;

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         */
        virtual void applyViewport() const;

        /**
         * Apply the frustum matrix for the current rendering task.
         */
        virtual void applyFrustum() const;

        /** 
         * Apply the modelling transformation to position and orient the view
         * frustum.
         */
        virtual void applyHeadTransform() const;
        //*}

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in channel.cpp
        enum IAttribute
        {
            IATTR_HINT_STATISTICS,
            IATTR_ALL
        };
        
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //*}
#if 0
        /** @name Scene Object Access. */
        //*{
        SceneObject* getNextSceneObject();
        SceneObject* checkNextSceneObject();
        //void putSceneObject( SceneObject* object );
        void passSceneObject( SceneObject* object );
        void flushSceneObjects();
        //*}
#endif

    protected:
        /**
         * Destructs the channel.
         */
        virtual ~Channel();
        friend class Window;

        /** @name Actions */
        //*{
        /** 
         * Start a frame by unlocking all child resources.
         * 
         * @param frameNumber the frame to start.
         */
        void startFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Signal the completion of a frame to the parent.
         * 
         * @param frameNumber the frame to end.
         */
        void releaseFrame( const uint32_t frameNumber ) { /* currently nop */ }

        /** 
         * Release the local synchronization of the parent for a frame.
         * 
         * @param frameNumber the frame to release.
         */
        void releaseFrameLocal( const uint32_t frameNumber ) { /* nop */ }
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //*{
        /** 
         * Initialise this channel.
         * 
         * @param initID the init identifier.
         */
        virtual bool configInit( const uint32_t initID ){ return true; }

        /** 
         * Exit this channel.
         */
        virtual bool configExit(){ return true; }

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of channel-specific data. This method has to call startFrame().
         *
         * @param frameID the per-frame identifier.
         * @param frameNumber the frame to start.
         * @sa Config::beginFrame()
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
         */
        virtual void frameDrawFinish( const uint32_t frameID, 
                                      const uint32_t frameNumber )
            { releaseFrameLocal( frameNumber ); }

        /** 
         * Clear the frame buffer.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void frameClear( const uint32_t frameID );

        /** 
         * Draw the scene.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void frameDraw( const uint32_t frameID );

        /** 
         * Assemble input frames.
         * 
         * @param frameID the per-frame identifier.
         * @sa getInputFrames
         */
        virtual void frameAssemble( const uint32_t frameID );

        /** 
         * Read back the rendered scene.
         * 
         * @param frameID the per-frame identifier.
         * @sa getOutputFrames
         */
        virtual void frameReadback( const uint32_t frameID );

        /**
         * Setup the OpenGL state for a readback or assemble operation.
         *
         * The default implementation is very conservative and saves any state
         * which is potentially changed by the assembly routines.
         */
        virtual void setupAssemblyState();

        /**
         * Reset the OpenGL state after an assembly operation.
         */
        virtual void resetAssemblyState();
        //*}

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}
    private:
        /** The parent window. */
        Window* const _window;

        /** The name. */
        std::string    _name;
        
        /** A unique color assigned by the server during config init. */
        vmml::Vector3ub _color;

        /** The reason for the last error. */
        std::string     _error;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** server-supplied rendering data. */
        RenderContext* _context;

        /** server-supplied vector of output frames for current task. */
        std::vector<Frame*> _outputFrames;

        /** server-supplied vector of input frames for current task. */
        std::vector<Frame*> _inputFrames;

        /** The native pixel viewport wrt the window. */
        eq::PixelViewport _pvp;

        /** The native viewport. */
        Viewport       _vp;

        /** The native ('identity') frustum. */
        vmml::Frustumf  _frustum;

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

        /** Setup the current rendering context. */
        void _setRenderContext( RenderContext& context );

        /* The command handler functions. */
        eqNet::CommandResult _cmdConfigInit( eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExit( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameStart( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameFinish( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameClear( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameDraw( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameDrawFinish( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameAssemble( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameReadback( eqNet::Command& command );
        eqNet::CommandResult _cmdFrameTransmit( eqNet::Command& command );
    };
}

#endif // EQ_CHANNEL_H

