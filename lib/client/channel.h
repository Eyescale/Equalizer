/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include <eq/client/commands.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/window.h>

#include <eq/net/base.h>
#include <eq/net/object.h>
#include <eq/vmmlib/VMMLib.h>

namespace eq
{
    class Channel;
    class Frame;
    class Node;
    class Range;
    class RenderContext;
    class SceneObject;

    class Channel : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new channel.
         */
        Channel();

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
        eqBase::RefPtr<eqNet::Node> getServer() const 
            { return ( _window ? _window->getServer() : 0 );}

        const std::string& getName() const { return _name; }

        /** 
         * Set the near and far planes for this channel.
         * 
         * The near and far planes are set during initialisation and are
         * inherited by source channels contributing to the rendering of this
         * channel. Dynamic near and far planes can be applied using
         * applyNearFar.
         *
         * @param near the near plane.
         * @param far the far plane.
         */
        void setNearFar( const float near, const float far );

        /** 
         * Returns the current near and far planes for this channel.
         *
         * The current near and far plane depends on the context from which this
         * function is called.
         * 
         * @param near a pointer to store the near plane.
         * @param far a pointer to store the far plane.
         */
        void getNearFar( float *near, float *far );
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
        virtual bool init( const uint32_t initID ){ return true; }

        /** 
         * Exit this channel.
         */
        virtual bool exit(){ return true; }

        /** 
         * Clear the frame buffer.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void clear( const uint32_t frameID );

        /** 
         * Draw the scene.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void draw( const uint32_t frameID );

        /** 
         * Assemble input frames.
         * 
         * @param frameID the per-frame identifier.
         * @sa getInputFrames
         */
        virtual void assemble( const uint32_t frameID );

        /** 
         * Read back the rendered scene.
         * 
         * @param frameID the per-frame identifier.
         * @sa getOutputFrames
         */
        virtual void readback( const uint32_t frameID );

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

        /**
         * @name Operations
         *
         * Operations are only available from within certain callbacks.
         */
        //*{

        /** 
         * Apply the current rendering buffer.
         */
        void applyBuffer();

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         */
        void applyViewport();

        /**
         * Apply the frustum matrix for the current rendering task.
         */
        void applyFrustum() const;

        /** 
         * Apply the modelling transformation to position and orient the view
         * frustum.
         */
        void applyHeadTransform() const;
        //*}

        /**
         * @name Context-specific data access.
         * 
         * The data returned by these methods depends on the context (callback)
         * they are called from, typically the data for the current rendering
         * task.
         */
        //*{
        /** @return the channel's current pixel viewport. */
        const PixelViewport& getPixelViewport() const;

        /** @return the view frustum for the current rendering task. */
        const vmml::Frustumf& getFrustum() const;

        /** @return the database range for the current rendering task. */
        const Range& getRange() const;

        /**
         * @return the modelling transformation to position and orient the view
         *         frustum.
         */
        const vmml::Matrix4f& getHeadTransform() const;

        /** @return the list of input frames, used from assemble(). */
        const std::vector<Frame*>& getInputFrames() { return _inputFrames; }

        /** @return the list of output frames, used from readback(). */
        const std::vector<Frame*>& getOutputFrames() { return _outputFrames; }
        //*}

        /** @name Scene Object Access. */
        //*{
        SceneObject* getNextSceneObject();
        SceneObject* checkNextSceneObject();
        //void putSceneObject( SceneObject* object );
        void passSceneObject( SceneObject* object );
        void flushSceneObjects();
        //*}

    protected:
        /**
         * Destructs the channel.
         */
        virtual ~Channel();

    private:
        /** The parent node. */
        friend class   Window;
        Window*        _window;

        /** The name. */
        std::string    _name;

        /** server-supplied rendering data. */
        RenderContext *_context;

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
        
        /* The command handler functions. */
        eqNet::CommandResult _pushCommand( eqNet::Command& command );
        eqNet::CommandResult _reqInit( eqNet::Command& command );
        eqNet::CommandResult _reqExit( eqNet::Command& command );
        eqNet::CommandResult _reqClear( eqNet::Command& command );
        eqNet::CommandResult _reqDraw( eqNet::Command& command );
        eqNet::CommandResult _reqAssemble( eqNet::Command& command );
        eqNet::CommandResult _reqReadback( eqNet::Command& command );
        eqNet::CommandResult _reqTransmit( eqNet::Command& command );
    };
}

#endif // EQ_CHANNEL_H

