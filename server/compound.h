
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

#ifndef EQSERVER_COMPOUND_H
#define EQSERVER_COMPOUND_H

#include "channel.h"               // used in inline method
#include "frustum.h"               // member
#include "frustumData.h"           // member

#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/global.h>
#include <eq/client/projection.h>
#include <eq/client/task.h>
#include <eq/client/wall.h>
#include <eq/fabric/range.h>    // member
#include <eq/fabric/subPixel.h> // member
#include <eq/fabric/viewport.h> // member
#include <eq/fabric/zoom.h>     // member
#include <eq/net/barrier.h>
#include <eq/base/thread.h>
#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class CompoundListener;
    class CompoundVisitor;
    class Frame;
    class SwapBarrier;
    
    /**
     * The compound.
     */
    class Compound
    {
    public:
        /** 
         * Constructs a new Compound.
         */
        EQSERVER_EXPORT Compound();

        /**
         * Constructs a new, deep copy of the passed compound
         */
        Compound( const Compound& from, Config* config, Compound* parent );

        /** Destruct the compound and all children. */
        virtual ~Compound();

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

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in compound.cpp
        enum IAttribute
        {
            IATTR_STEREO_MODE,
            IATTR_STEREO_ANAGLYPH_LEFT_MASK,
            IATTR_STEREO_ANAGLYPH_RIGHT_MASK,
            IATTR_HINT_OFFSET,
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        /**
         * @name Data Access
         */
        //@{
        /** 
         * Adds a new child to this compound.
         * 
         * @param child the child.
         */
        EQSERVER_EXPORT void addChild( Compound* child );

        /** 
         * Removes a child from this compound.
         * 
         * @param child the child
         * @return <code>true</code> if the child was removed, 
         *         <code>false</code> otherwise.
         */
        EQSERVER_EXPORT bool removeChild( Compound* child );

        /** @return if the compound is a leaf compound. */
        bool isLeaf() const { return _children.empty(); }

        /** @return if the compound has the destination channel. */
        bool isDestination() const;
        
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

        Config*       getConfig()       { return getRoot()->_config; }
        const Config* getConfig() const { return getRoot()->_config; }

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
        EQSERVER_EXPORT void setChannel( Channel* channel );

        /** 
         * Return the channel of this compound.
         * 
         * Note that the channel is inherited, that is, if this compound has no
         * channel, the parent's channel is returned.
         *
         * @return the channel of this compound.
         */
        EQSERVER_EXPORT Channel* getChannel();
        EQSERVER_EXPORT const Channel* getChannel() const;

        Window* getWindow();
        const Window* getWindow() const;

        Pipe* getPipe();
        const Pipe* getPipe() const;

        /** @return the frustum of this compound. */
        Frustum& getFrustum() { return _frustum; }

        /** Attach a load balancer to this compound. */
        EQSERVER_EXPORT void addEqualizer( Equalizer* equalizer );

        /** Get the attached load balancers. */
        const EqualizerVector& getEqualizers() const
            { return _equalizers; }

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
         * @param task the compound task to add.
         */
        void enableTask( const eq::Task task ) { _data.tasks |= task; }

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
         * @param buffer the compound image buffer to add.
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

        void setSubPixel( const eq::SubPixel& subpixel )
            { _data.subpixel = subpixel; }
        const eq::SubPixel& getSubPixel() const    { return _data.subpixel; }

        void setZoom( const eq::Zoom& zoom )       { _data.zoom = zoom; }
        const eq::Zoom& getZoom() const            { return _data.zoom; }

        void setMaxFPS( const float fps )          { _data.maxFPS = fps; }
        float getMaxFPS() const                    { return _data.maxFPS; }

        void setUsage( const float usage )         { _usage = usage; }
        float getUsage() const                     { return _usage; }

        void setTaskID( const uint32_t id )        { _taskID = id; }
        uint32_t getTaskID() const                 { return _taskID; }
        //@}

        /** @name IO object access. */
        //@{
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
        EQSERVER_EXPORT void addInputFrame( Frame* frame );

        /** @return the vector of input frames. */
        const FrameVector& getInputFrames() const {return _inputFrames; }

        /** 
         * Add a new output frame for this compound.
         *
         * @param frame the output frame.
         */
        EQSERVER_EXPORT void addOutputFrame( Frame* frame );

        /** @return the vector of output frames. */
        const FrameVector& getOutputFrames() const { return _outputFrames; }
        //@}

        /** 
         * @name Inherit data access needed during channel update.
         * 
         * Inherit data are the actual, as opposed to configured, attributes and
         * data used by the compound. The inherit data is updated at the
         * beginning of each update().
         */
        //@{
        uint32_t getInheritBuffers() const { return _inherit.buffers; }
        const eq::PixelViewport& getInheritPixelViewport() const 
            { return _inherit.pvp; }
        const Vector4i& getInheritOverdraw() const
            { return _inherit.overdraw; }
        const eq::Viewport& getInheritViewport() const { return _inherit.vp; }
        const eq::Range& getInheritRange()   const { return _inherit.range; }
        const eq::Pixel& getInheritPixel()   const { return _inherit.pixel; }
        const eq::SubPixel& getInheritSubPixel() const 
            { return _inherit.subpixel; }
        const eq::Zoom& getInheritZoom()     const { return _inherit.zoom; }
        uint32_t getInheritPeriod()          const { return _inherit.period; }
        uint32_t getInheritPhase()           const { return _inherit.phase; }
        float getInheritMaxFPS()             const { return _inherit.maxFPS; }
        int32_t  getInheritIAttribute( const IAttribute attr ) const
            { return _inherit.iAttributes[attr]; }
        const FrustumData& getInheritFrustumData() const 
            { return _inherit.frustumData; }
        uint32_t getInheritTasks()           const { return _inherit.tasks; }
        uint32_t getInheritEyes()            const { return _inherit.eyes; }
        const Channel* getInheritChannel()   const { return _inherit.channel; }
        
        /** @return true if the task is set, false if not. */
        bool testInheritTask( const eq::Task task ) const
            { return (_inherit.tasks & task); }

        /** Delete an inherit task, if it was set. */
        void unsetInheritTask( const eq::Task task )
            { _inherit.tasks &= ~task; }

        /** @return true if the eye pass is used, false if not. */
        bool testInheritEye( const eq::Eye eye ) const
            { return ( _inherit.eyes & (1<<eye) ); }
        //@}

        /**
         * @name Frustum Operations
         */
        //@{
        /** 
         * Set the compound's frustum using a wall description.
         * 
         * @param wall the wall description.
         */
        EQSERVER_EXPORT void setWall( const eq::Wall& wall );
        
        /** @return the last specified wall description. */
        const eq::Wall& getWall() const { return _frustum.getWall(); }

        /** 
         * Set the compound's frustum using a projection description
         * 
         * @param projection the projection description.
         */
        EQSERVER_EXPORT void setProjection( const eq::Projection& projection );

        /** @return the last specified projection description. */
        const eq::Projection& getProjection() const 
            { return _frustum.getProjection(); }

        /** @return the type of the latest specified frustum. */
        eq::Frustum::Type getFrustumType() const
            { return _frustum.getCurrentType(); }

        /** @return the frustum. */
        const eq::Frustum& getFrustum() const { return _frustum; }

        /** Update the frustum from the view or segment. */
        void updateFrustum();

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
        //@}

        /** @name Compound Operations. */
        //@{
        /** 
         * Traverse the compound and all children using a compound visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( CompoundVisitor& visitor ) const;
        /** Non-const version of accept(). */
        EQSERVER_EXPORT VisitorResult accept( CompoundVisitor& visitor );


        /** Activate the compound tree. */
        void activate();

        /** Deactivate the compound tree. */
        void deactivate();

        /** Set the active state of this compound only. */
        void setActive( const bool active ) 
            { EQASSERT( _active != active ); _active = active; }

        /** @return if the compound is activated and current (DPlex). */
        bool isActive() const;

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
        //@}

        /** @name Compound listener interface. */
        //@{
        /** Register a compound listener. */
        void addListener( CompoundListener* listener );
        /** Deregister a compound listener. */
        void removeListener( CompoundListener* listener );

        /** Notify all listeners that the compound is about to be updated. */
        void fireUpdatePre( const uint32_t frameNumber );
        //@}
        
        /**
         * @name Attributes
         */
        //@{
        void setIAttribute( const IAttribute attr, const int32_t value )
            { _data.iAttributes[attr] = value; }
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _data.iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //@}

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

        /** Has been activated (by layout) */
        bool _active;

        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** Percentage the resource should be used. */
        float _usage;

        /** Unique identifier for channel tasks. */
        uint32_t _taskID;

        struct InheritData
        {
            InheritData();

            Channel*          channel;
            eq::Viewport      vp;
            eq::PixelViewport pvp;
            Vector4i          overdraw;
            eq::Range         range;
            eq::Pixel         pixel;
            eq::SubPixel      subpixel;
            FrustumData       frustumData;
            eq::Zoom          zoom;
            uint32_t          buffers;
            uint32_t          eyes;
            uint32_t          tasks;
            uint32_t          period;
            uint32_t          phase;
            int32_t           iAttributes[IATTR_ALL];
            float             maxFPS;
            bool              active;

            union // placeholder for binary-compatible changes
            {
                char dummy[16];
            };
        };

        InheritData _data;
        InheritData _inherit;

        /** The frustum description of this compound. */
        Frustum _frustum;

        typedef std::vector< CompoundListener* > CompoundListeners;
        CompoundListeners _listeners;

        EqualizerVector _equalizers;

        SwapBarrier* _swapBarrier;

        FrameVector _inputFrames;
        FrameVector _outputFrames;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        CHECK_THREAD_DECLARE( _serverThread );

        //-------------------- Methods --------------------
        void _updateOverdraw( Wall& wall );
        void _updateInheritPVP();
        void _updateInheritOverdraw();

        void _setDefaultFrameName( Frame* frame );

        void _fireChildAdded( Compound* child );
        void _fireChildRemove( Compound* child );
    };

    std::ostream& operator << ( std::ostream& os, const Compound* compound );
}
}
#endif // EQSERVER_COMPOUND_H
