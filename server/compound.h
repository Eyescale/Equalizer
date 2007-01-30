
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include "channel.h"

#include <eq/client/colorMask.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/projection.h>
#include <eq/client/range.h>
#include <eq/client/renderContext.h>
#include <eq/client/view.h>
#include <eq/client/viewport.h>
#include <eq/client/wall.h>
#include <eq/client/global.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Frame;
    class SwapBarrier;
    
    enum TraverseResult
    {
        TRAVERSE_CONTINUE,
        TRAVERSE_TERMINATE
    };

    /**
     * The compound.
     */
    class Compound
    {
    public:
        /** 
         * Constructs a new Compound.
         */
        Compound();

        /**
         * Constructs a new, deep copy of the passed compound
         */
        Compound( const Compound& from );

        /**
         * Compound tasks define the actions executed by a compound on its
         * channel during compound update, in the order they are defined.
         *
         * The enums are spaced apart to leave room for future additions without
         * breaking binary backward compatibility.
         */
        enum Task
        {
            TASK_NONE     = 0,
            TASK_DEFAULT  = 0x1,      //!< ALL for leaf, else ASSEMBLE|READBACK
            TASK_CLEAR    = 0x10,     //!< Clear the framebuffer
            TASK_CULL     = 0x100,    //!< Cull data
            TASK_DRAW     = 0x1000,   //!< Draw data to the framebuffer
            TASK_ASSEMBLE = 0x10000,  //!< Combine input frames
            TASK_READBACK = 0x100000, //!< Read results to output frames
            TASK_ALL      = 0xfffffff
        };

        /**
         * The cyclop eye is the standard for monoscopic rendering. It is
         * also selected if no eye is set. The left and right eye are used
         * for stereo rendering and are computed from the cyclop eye and the
         * eye offset.
         * The enums allow bitwise OR operations.
         */
        enum Eye
        {
            EYE_UNDEFINED = 0,       //!< use inherited default eye(s)
            EYE_CYCLOP    = 0x01,    //!< render monoscopic 'middle' eye
            EYE_LEFT      = 0x02,    //!< render left eye
            EYE_RIGHT     = 0x04     //!< render right eye
        };

        /** The color mask bits, used for anaglyphic stereo. */
        enum ColorMask
        {
            COLOR_MASK_NONE      = 0,
            COLOR_MASK_RED       = 0x02,
            COLOR_MASK_GREEN     = 0x04,
            COLOR_MASK_BLUE      = 0x08,
            COLOR_MASK_ALL       = 0xff
        };

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in compound.cpp
        enum IAttribute
        {
            IATTR_STEREO_MODE,
            IATTR_STEREO_ANAGLYPH_LEFT_MASK,
            IATTR_STEREO_ANAGLYPH_RIGHT_MASK,
            IATTR_ALL
        };

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Adds a new child to this compound.
         * 
         * @param child the child.
         */
        void addChild( Compound* child );

        /** 
         * Removes a child from this compound.
         * 
         * @param child the child
         * @return <code>true</code> if the child was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeChild( Compound* child );

        /** 
         * Returns the number of children on this compound.
         * 
         * @return the number of children on this compound. 
         */
        uint32_t nChildren() const { return _children.size(); }

        /** @return if the compound is a leaf compound. */
        bool isLeaf() const { return _children.empty(); }

        /** 
         * Gets a child.
         * 
         * @param index the child's index. 
         * @return the child.
         */
        Compound* getChild( const uint32_t index ) const
            { return _children[index]; }

        /** @return the parent compound. */
        Compound* getParent() const
            { return _parent; }

        /** @return the root of the compound tree. */
        Compound* getRoot()
            { return _parent ? _parent->getRoot() : this; }

        Config* getConfig()
            { return getRoot()->_config; }
        Node* getNode()
            { return getChannel()->getNode(); }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the channel of this compound.
         *
         * The compound uses the channel for all rendering operations executed
         * by this compound.
         * 
         * @param channel the channel.
         */
        void setChannel( Channel* channel ){ _data.channel = channel; }

        /** 
         * Return the channel of this compound.
         * 
         * Note that the channel is inherited, that is, if this compound has no
         * channel, the parent's channel is returned.
         *
         * @return the channel of this compound.
         */
        Channel* getChannel() const;

        Window* getWindow() const;

        /** 
         * Set the tasks to be executed by the compound, overwriting previous
         * tasks.
         *
         * Tasks define which actions are executed by the compound, and provide
         * a flexible way of configuring the decomposition and recomposition. A
         * separate html design document describes them in depth.
         * 
         * @param tasks the compound tasks.
         */
        void setTasks( const uint32_t tasks ) { _data.tasks = tasks; }

        /** 
         * Add a task to be executed by the compound, preserving previous tasks.
         * 
         * @param tasks the compound tasks.
         */
        void enableTask( const Task task ) { _data.tasks |= task; }

        /** @return the tasks executed by this compound. */
        uint32_t getTasks() const { return _data.tasks; }

        /** 
         * Set the image buffers to be used by the compound during
         * recomposition, overwriting previous buffers.
         *
         * @param buffers the compound image buffers.
         */
        void setBuffers( const uint32_t buffers ) { _data.buffers = buffers; }

        /** 
         * Add a image buffer to be used by the compound, preserving previous
         * buffers.
         * 
         * @param buffers the compound image buffers.
         */
        void enableBuffer( const eq::Frame::Buffer buffer )
            { _data.buffers |= buffer; }

        /** @return the image buffers used by this compound. */
        uint32_t getBuffers() const { return _data.buffers; }

        void setViewport( const eq::Viewport& vp ) { _data.vp = vp; }
        const eq::Viewport& getViewport() const { return _data.vp; }

        void setRange( const eq::Range& range ) { _data.range = range; }
        const eq::Range& getRange() const { return _data.range; }
        //*}

        /** @name IO object access. */
        //*{
        /** 
         * Set a swap barrier.
         *
         * Windows of compounds with the same swap barrier name will enter a
         * barrier before executing eq::Window::swap. Setting an empty string
         * disables the swap barrier.
         * 
         * @param barrier the swap barrier.
         */
        void setSwapBarrier( SwapBarrier* barrier );
        
        /** @return the current swap barrier. */
        SwapBarrier* getSwapBarrier() const { return _swapBarrier; }

        /** 
         * Add a new input frame for this compound.
         *
         * @param frame the input frame.
         */
        void addInputFrame( Frame* frame );

        /** @return the vector of input frames. */
        const std::vector<Frame*>& getInputFrames() const {return _inputFrames;}

        /** 
         * Add a new output frame for this compound.
         *
         * @param frame the output frame.
         */
        void addOutputFrame( Frame* frame );

        /** @return the vector of output frames. */
        const std::vector<Frame*>& getOutputFrames() const
            { return _outputFrames; }
        //*}

        /**
         * @name View Operations
         */
        //*{
        /** 
         * Set the compound's view using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const eq::Wall& wall );
        
        /** @return the last specified wall description. */
        const eq::Wall& getWall() const { return _view.wall; }

        /** 
         * Set the compound's view using a projection description
         * 
         * @param projection the projection description.
         */
        void setProjection( const eq::Projection& projection );

        /** @return the last specified projection description. */
        const eq::Projection& getProjection() const { return _view.projection; }

        /** 
         * Set the compound's view as a four-by-four matrix.
         * 
         * @param view the view description.
         */
        void setView( const eq::View& view  );

        /** @return the last specified projection matrix. */
        const eq::View& getView() const { return _view.view; }

        /** @return the bitwise OR of the eye values. */
        const uint32_t getEyes() const { return _data.eyes; }

         /** 
         * Set the eyes to be used by the compound.
         * 
         * Previously set eyes are overwritten.
         *
         * @param eyes the compound eyes.
         */
        void setEyes( const uint32_t eyes ) { _data.eyes = eyes; }

        /** 
         * Add eyes to be used by the compound.
         *
         * Previously set eyes are preserved.
         * 
         * @param eyes the compound eyes.
         */
        void enableEye( const uint32_t eyes ) { _data.eyes |= eyes; }
        //*}

        /** @name Compound Operations. */
        //*{
        typedef TraverseResult (*TraverseCB)(Compound* compound,void* userData);

        /** 
         * Traverses a compound tree in a top-down, left-to-right order.
         * 
         * @param compound the top level compound of the tree to traverse.
         * @param preCB the callback to execute for non-leaf compounds when
         *              traversing down, can be <code>NULL</code>.
         * @param leafCB the callback to execute for leaf compounds, can be
         *               <code>NULL</code>
         * @param postCB the callback to execute for non-leaf compounds when
         *              traversing up, can be <code>NULL</code>.
         * @param userData an opaque pointer passed through to the callbacks.
         *
         * @return the result of the traversal, <code>TRAVERSE_TERMINATE</code>
         *         if the traversal was terminated by one of the callbacks, 
         *         otherwise <code>TRAVERSE_CONTINUE</code> .
         */
        static TraverseResult traverse( Compound* compound, TraverseCB preCB,
                                        TraverseCB leafCB, TraverseCB postCB,
                                        void* userData );

        /** 
         * Initialises this compound.
         */
        void init();

        /** 
         * Exits this compound.
         */
        void exit();

        /** 
         * Updates this compound.
         * 
         * The compound's parameters for the next frame are computed.
         */
        void update();

        /** 
         * Update a channel by generating all rendering tasks for this frame.
         * 
         * @param channel the channel to update.
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void updateChannel( Channel* channel, const uint32_t frameID );
        //*}

        /**
         * @name Attributes
         */
        //*{
        void setIAttribute( const IAttribute attr, const int32_t value )
            { _data.iAttributes[attr] = value; }
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _data.iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //*}

        /** 
         * @name Inherit Data Access.
         * 
         * Inherit data are the actual, as opposed to configured, attributes and
         * data used by the compound. The inherit data is updated at the
         * beginning of each update().
         */
        //*{
        uint32_t getInheritBuffers() const { return _inherit.buffers; }

        /** @return true if the task is set, false if not. */
        bool testInheritTask( const Task task ) const
            { return (_inherit.tasks & task); }
        //*}

    protected:
        virtual ~Compound();

    private:
        std::string _name;
        
        /** 
         * The config the compound is attached to, only set on the root 
         * compound.
         */
        friend class Config;
        Config* _config;

        Compound               *_parent;
        std::vector<Compound*>  _children;
        
        Compound* _getNext() const;

        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        struct View
        {
            enum Type
            {
                NONE,
                WALL,
                PROJECTION,
                VIEW
            };

            View() : latest( NONE ) {}

            Type           latest;
            eq::Wall       wall;
            eq::Projection projection;
            eq::View       view;
        } 
        _view;

        struct InheritData
        {
            InheritData();

            Channel*          channel;
            eq::Viewport      vp;
            eq::PixelViewport pvp;
            eq::Range         range;
            eq::View          view;
            uint32_t          buffers;
            uint32_t          eyes;
            uint32_t          tasks;            
            int32_t           iAttributes[IATTR_ALL];
        };

        InheritData _data;
        InheritData _inherit;

        SwapBarrier* _swapBarrier;

        std::vector<Frame*> _inputFrames;
        std::vector<Frame*> _outputFrames;
        
        void _setDefaultFrameName( Frame* frame );

        static TraverseResult _initCB( Compound* compound, void* );

        //----- pre-render compound setup
        struct UpdateData
        {
            stde::hash_map<std::string, eqNet::Barrier*> swapBarriers;
            stde::hash_map<std::string, Frame*>          outputFrames;
        };

        static TraverseResult _updatePreCB( Compound* compound, void* );
        static TraverseResult _updateCB( Compound* compound, void* );
        static TraverseResult _updatePostCB( Compound* compound, void* );
        void _updateInheritData();
        void _updateSwapBarriers( UpdateData* data );
        void _updateOutput( UpdateData* data );
        static TraverseResult _updateInputCB( Compound* compound, void* );
        void _updateInput( UpdateData* data );

        //----- per-channel render task generation
        struct UpdateChannelData
        {
            Channel* channel;
            uint32_t frameID;
            Eye      eye;
        };

        static TraverseResult _updatePreDrawCB(Compound* compound, void* );
        static TraverseResult _updateDrawCB(Compound* compound, void* );
        static TraverseResult _updatePostDrawCB( Compound* compound, void* );
        void _updatePostDraw( const eq::RenderContext& context );
        void   _updateAssemble( const eq::RenderContext& context );
        void   _updateReadback( const eq::RenderContext& context );
        void _setupRenderContext( eq::RenderContext& context, 
                                  const UpdateChannelData* data );
        GLenum _getDrawBuffer( const UpdateChannelData* data );
        eq::ColorMask _getDrawBufferMask( const UpdateChannelData* data );
        void _computeFrustum( eq::RenderContext& context, const Eye whichEye );

        friend std::ostream& operator << ( std::ostream& os,
                                           const Compound* compound );
    };

    std::ostream& operator << ( std::ostream& os,const Compound* compound );
};
#endif // EQS_COMPOUND_H
