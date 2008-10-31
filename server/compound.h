
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUND_H
#define EQSERVER_COMPOUND_H

#include "channel.h"               // used in inline method
#include "view.h"                  // member
#include "viewData.h"              // member

#include <eq/client/colorMask.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/global.h>
#include <eq/client/projection.h>
#include <eq/client/range.h>
#include <eq/client/renderContext.h>
#include <eq/client/viewport.h>
#include <eq/client/wall.h>
#include <eq/base/thread.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class ConstCompoundVisitor;
    class CompoundListener;
    class CompoundVisitor;
    class Frame;
    class LoadBalancer;
    class SwapBarrier;
    
    /**
     * The compound.
     */
    class EQSERVER_EXPORT Compound
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

        /** Destruct the compound and all children. */
        virtual ~Compound();

        /**
         * Compound tasks define the actions executed by a compound on its
         * channel during compound update, in the order they are defined.
         *
         * The enums are spaced apart to leave room for future additions without
         * breaking binary backward compatibility.
         */
        enum Task
        {
            TASK_NONE     = EQ_BIT_NONE,
            TASK_DEFAULT  = EQ_BIT1,   //!< ALL for leaf, else ASSEMBLE|READBACK
            TASK_CLEAR    = EQ_BIT5,   //!< Clear the framebuffer
            TASK_CULL     = EQ_BIT9,   //!< Cull data
            TASK_DRAW     = EQ_BIT13,  //!< Draw data to the framebuffer
            TASK_ASSEMBLE = EQ_BIT17,  //!< Combine input frames
            TASK_READBACK = EQ_BIT21,  //!< Read results to output frames
            TASK_ALL      = EQ_BIT_ALL
        };

        /**
         * Eye pass bit mask for which the compound is enabled.
         */
        enum EyeMask
        {
            EYE_UNDEFINED  = 0,                 //!< use inherited eye(s)
            EYE_CYCLOP_BIT = 1<<eq::EYE_CYCLOP, //!<  monoscopic 'middle' eye
            EYE_LEFT_BIT   = 1<<eq::EYE_LEFT,   //!< left eye
            EYE_RIGHT_BIT  = 1<<eq::EYE_RIGHT   //!< right eye
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

        /** The return value of a CompoundVisitor::visit method */
        enum VisitorResult
        {
            TRAVERSE_CONTINUE,
            TRAVERSE_TERMINATE,
            TRAVERSE_PRUNE
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
            IATTR_HINT_OFFSET,
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

        /** @return if the compound is a leaf compound. */
        bool isLeaf() const { return _children.empty(); }

        /** @return if the compound is active. */
        bool isActive() const { return _inherit.active; }
        
        /** @return the children of this compound. */
        const CompoundVector& getChildren() const { return _children; }

        /** @return the parent compound. */
        Compound* getParent() const
            { return _parent; }

        /** @return the root of the compound tree. */
        Compound* getRoot()
            { return _parent ? _parent->getRoot() : this; }
        const Compound* getRoot() const
            { return _parent ? _parent->getRoot() : this; }

        /** @return the next sibling, or 0. */
        Compound* getNext() const;

        Config* getConfig()
            { return getRoot()->_config; }
        Node* getNode();

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
        void setChannel( Channel* channel );

        /** 
         * Return the channel of this compound.
         * 
         * Note that the channel is inherited, that is, if this compound has no
         * channel, the parent's channel is returned.
         *
         * @return the channel of this compound.
         */
        Channel* getChannel();
        const Channel* getChannel() const;

        Window* getWindow();
        const Window* getWindow() const;

        /** @return the view of this compound. */
        View& getView() { return _view; }

        /** Attach a load balancer to this compound. */
        void setLoadBalancer( LoadBalancer* loadBalancer );

        /** Get the attached load balancer. */
        const LoadBalancer* getLoadBalancer() const { return _loadBalancer; }
        LoadBalancer*       getLoadBalancer()       { return _loadBalancer; }

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
        const eq::Viewport& getViewport() const    { return _data.vp; }

        void setRange( const eq::Range& range )    { _data.range = range; }
        const eq::Range& getRange() const          { return _data.range; }

        void setPeriod( const uint32_t period )    { _data.period = period; }
        uint32_t getPeriod() const                 { return _data.period; }

        void setPhase( const uint32_t phase )      { _data.phase = phase; }
        uint32_t getPhase() const                  { return _data.phase; }

        void setPixel( const eq::Pixel& pixel )    { _data.pixel = pixel; }
        const eq::Pixel& getPixel() const          { return _data.pixel; }

        void setMaxFPS( const float fps )          { _data.maxFPS = fps; }
        float getMaxFPS() const                    { return _data.maxFPS; }
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
        const SwapBarrier* getSwapBarrier() const { return _swapBarrier; }

        /** 
         * Add a new input frame for this compound.
         *
         * @param frame the input frame.
         */
        void addInputFrame( Frame* frame );

        /** @return the vector of input frames. */
        const FrameVector& getInputFrames() const {return _inputFrames;}

        /** 
         * Add a new output frame for this compound.
         *
         * @param frame the output frame.
         */
        void addOutputFrame( Frame* frame );

        /** @return the vector of output frames. */
        const FrameVector& getOutputFrames() const
            { return _outputFrames; }
        //*}

        /** 
         * @name Inherit data access needed during channel update.
         * 
         * Inherit data are the actual, as opposed to configured, attributes and
         * data used by the compound. The inherit data is updated at the
         * beginning of each update().
         */
        //*{
        uint32_t getInheritBuffers() const { return _inherit.buffers; }
        const eq::PixelViewport& getInheritPixelViewport() const 
            { return _inherit.pvp; }
        const eq::Viewport& getInheritViewport() const { return _inherit.vp; }
        const eq::Range& getInheritRange()   const { return _inherit.range; }
        const eq::Pixel&  getInheritPixel()  const { return _inherit.pixel; }
        uint32_t getInheritPeriod()          const { return _inherit.period; }
        float getInheritMaxFPS()             const { return _inherit.maxFPS; }
        uint32_t getInheritTask()            const { return _inherit.tasks; }
        int32_t  getInheritIAttribute( const IAttribute attr ) const
            { return _inherit.iAttributes[attr]; }
        const ViewData& getInheritViewData() const { return _inherit.viewData; }
        uint32_t getInheritTasks()           const { return _inherit.tasks; }
        uint32_t getInheritEyes()            const { return _inherit.eyes; }
        const Channel* getInheritChannel()   const { return _inherit.channel; }

        const vmml::Vector2i& getInheritScreenOrigin() const
            { return _inherit.screen.origin; }
        const vmml::Vector2i& getInheritScreenSize() const
            { return getRoot()->_screens.find( _inherit.screen.id )->second; }

        /** @return true if the task is set, false if not. */
        bool testInheritTask( const Task task ) const
            { return (_inherit.tasks & task); }
        bool testInheritEye( const eq::Eye eye ) const
            { return ( _inherit.eyes & (1<<eye) ); }
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
        const eq::Wall& getWall() const { return _view.getWall(); }

        /** 
         * Set the compound's view using a projection description
         * 
         * @param projection the projection description.
         */
        void setProjection( const eq::Projection& projection );

        /** @return the last specified projection description. */
        const eq::Projection& getProjection() const 
            { return _view.getProjection(); }

        /** @return the type of the latest specified view. */
        eq::View::Type getLatestView() const { return _view.getCurrentType(); }

        /** @return the bitwise OR of the eye values. */
        uint32_t getEyes() const { return _data.eyes; }

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

        /** Set the screen identifier. */
        void setScreen( const uint32_t id ) { _data.screen.id = id; }
        
        /** @return the screeen identifier. */
        uint32_t getScreen() const { return _data.screen.id; }

        /** Set the offset from the screen's origin. */
        void setScreenOrigin( const vmml::Vector2i& origin ) 
            { _data.screen.origin = origin; }

        /** @return the offset from the screen's origin. */
        const vmml::Vector2i& getScreenOrigin() const
            { return _data.screen.origin; }
        //*}

        /** @name Compound Operations. */
        //*{
        /** 
         * Traverse the compound and all (active) children using a compound
         * visitor.
         * 
         * @param visitor the visitor.
         * @param activeOnly traverse only the active DPlex (true), or all
         *                   (false) compounds.
         * 
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ConstCompoundVisitor* visitor,
                              const bool activeOnly ) const;
        /** Non-const version of accept(). */
        VisitorResult accept( CompoundVisitor* visitor,
                              const bool activeOnly );

        /** 
         * Initializes this compound.
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
        void update( const uint32_t frameNumber );

        /** Update the inherit data of this compound. */
        void updateInheritData( const uint32_t frameNumber );
        //*}

        /** @name Compound listener interface. */
        //*{
        /** Register a compound listener. */
        void addListener( CompoundListener* listener );
        /** Deregister a compound listener. */
        void removeListener( CompoundListener* listener );

        /** Notify all listeners that the compound is about to be updated. */
        void fireUpdatePre( const uint32_t frameNumber );
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

        typedef stde::hash_map<std::string, net::Barrier*> BarrierMap;
        typedef stde::hash_map<std::string, Frame*>        FrameMap;

    private:
        //-------------------- Members --------------------
        std::string _name;
        
        /** 
         * The config the compound is attached to, only set on the root 
         * compound.
         */
        friend class Config;
        Config* _config;

        Compound*       _parent;
        CompoundVector  _children;
        
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        struct Screen
        {
            Screen() : id( 1 ), origin( eq::AUTO, eq::AUTO ) {}

            uint32_t       id;
            vmml::Vector2i origin;
        };

        struct InheritData
        {
            InheritData();

            Channel*          channel;
            eq::Viewport      vp;
            eq::PixelViewport pvp;
            eq::Range         range;
            eq::Pixel         pixel;
            ViewData          viewData;
            Screen            screen;
            uint32_t          buffers;
            uint32_t          eyes;
            uint32_t          tasks;
            uint32_t          period;
            uint32_t          phase;
            float             maxFPS;
            int32_t           iAttributes[IATTR_ALL];
            bool              active;
        };

        InheritData _data;
        InheritData _inherit;

        /** The view description of this compound. */
        View _view;

        typedef std::vector< CompoundListener* > CompoundListeners;
        CompoundListeners _listeners;

        LoadBalancer* _loadBalancer;

        SwapBarrier* _swapBarrier;

        FrameVector _inputFrames;
        FrameVector _outputFrames;

        typedef stde::hash_map< uint32_t, vmml::Vector2i > ScreenMap;
        ScreenMap _screens;

        CHECK_THREAD_DECLARE( _serverThread );

        //-------------------- Methods --------------------
        void _setDefaultFrameName( Frame* frame );

        void _fireChildAdded( Compound* child );
        void _fireChildRemove( Compound* child );
    };

    std::ostream& operator << ( std::ostream& os, const Compound* compound );
}
}
#endif // EQSERVER_COMPOUND_H
