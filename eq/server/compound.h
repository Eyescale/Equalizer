
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_COMPOUND_H
#define EQSERVER_COMPOUND_H

#include "channel.h"               // used in inline method
#include "frustum.h"               // member
#include "frustumData.h"           // member
#include "visitorResult.h"         // enum
#include "types.h"

#include <eq/fabric/frame.h>      // enum Frame::Buffer
#include <eq/fabric/projection.h> // used in inline method
#include <eq/fabric/range.h>      // member
#include <eq/fabric/subPixel.h>   // member
#include <eq/fabric/swapBarrier.h> // RefPtr member
#include <eq/fabric/task.h>       // enum
#include <eq/fabric/viewport.h>   // member
#include <eq/fabric/wall.h>       // used in inline method
#include <eq/fabric/zoom.h>       // member
#include <co/barrier.h>
#include <lunchbox/thread.h>
#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
/** The compound form the tree describing the rendering task setup. */
class Compound
{
public:
    /** Construct a new root compound. */
    EQSERVER_API explicit Compound( Config* parent );

    /** Construct a new compound child. */
    EQSERVER_API explicit Compound( Compound* parent );

    /** Destruct the compound and all children. */
    virtual ~Compound();

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
        IATTR_FILL1,
        IATTR_FILL2,
        IATTR_ALL
    };

    /**
     * @name Data Access
     */
    //@{
    /** Reparent the given child to this compound. */
    EQSERVER_API void adopt( Compound* child );

    /** @return if the compound is a leaf compound. */
    bool isLeaf() const { return _children.empty(); }

    /** @return true if this is the top-most compound with a channel. */
    bool isDestination() const;

    /** @return if the compound has the destination channel. */
    bool hasDestinationChannel() const;

    /** @return the children of this compound. */
    const Compounds& getChildren() const { return _children; }

    /** @return the parent compound. */
    Compound* getParent() const { return _parent; }

    /** @return the root of the compound tree. */
    Compound* getRoot()
    { return _parent ? _parent->getRoot() : this; }
    const Compound* getRoot() const
    { return _parent ? _parent->getRoot() : this; }

    /** @return true if this is the top-most compound. */
    bool isRoot() const { return _parent==0; }

    /** @return the next sibling, or 0. */
    Compound* getNext() const;

    Config*       getConfig()       { return getRoot()->_config; }
    const Config* getConfig() const { return getRoot()->_config; }

    Node* getNode();
    ServerPtr getServer();

    void setName( const std::string& name ) { _name = name; }
    const std::string& getName() const      { return _name; }

    /**
     * Set the channel of this compound.
     *
     * The compound uses the channel for all rendering operations executed
     * by this compound. The compound's swap barrier will be updated with
     * the segment's swap barrier if this compound becomes a destination
     * compound.
     *
     * @param channel the channel.
     */
    EQSERVER_API void setChannel( Channel* channel );

    /**
     * Return the channel of this compound.
     *
     * Note that the channel is inherited, that is, if this compound has no
     * channel, the parent's channel is returned.
     *
     * @return the channel of this compound.
     */
    EQSERVER_API Channel* getChannel();
    EQSERVER_API const Channel* getChannel() const;

    Window* getWindow();
    const Window* getWindow() const;

    Pipe* getPipe();
    const Pipe* getPipe() const;

    /** Attach a load balancer to this compound. */
    EQSERVER_API void addEqualizer( Equalizer* equalizer );

    /** Get the attached load balancers. */
    const Equalizers& getEqualizers() const { return _equalizers; }

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
    void enableTask( const fabric::Task task ) { _data.tasks |= task; }

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
    void enableBuffer( const eq::fabric::Frame::Buffer buffer )
        { _data.buffers |= buffer; }

    /** @return the image buffers used by this compound. */
    uint32_t getBuffers() const { return _data.buffers; }

    void setViewport( const Viewport& vp ) { _data.vp = vp; }
    const Viewport& getViewport() const    { return _data.vp; }

    void setRange( const Range& range )    { _data.range = range; }
    const Range& getRange() const          { return _data.range; }

    void setPeriod( const uint32_t period )    { _data.period = period; }
    uint32_t getPeriod() const                 { return _data.period; }

    void setPhase( const uint32_t phase )      { _data.phase = phase; }
    uint32_t getPhase() const                  { return _data.phase; }

    void setPixel( const Pixel& pixel )    { _data.pixel = pixel; }
    const Pixel& getPixel() const          { return _data.pixel; }

    void setSubPixel( const SubPixel& subPixel )
        { _data.subPixel = subPixel; }
    const SubPixel& getSubPixel() const    { return _data.subPixel; }

    void setZoom( const Zoom& zoom )       { _data.zoom = zoom; }
    const Zoom& getZoom() const            { return _data.zoom; }

    void setMaxFPS( const float fps )          { _data.maxFPS = fps; }
    float getMaxFPS() const                    { return _data.maxFPS; }

    void setUsage( const float usage )
        { LBASSERT( usage >= 0.f ); _usage = usage; }
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
     * barrier before executing Window::swap. Setting an empty string
     * disables the swap barrier.
     *
     * @param barrier the swap barrier.
     */
    void setSwapBarrier( SwapBarrierPtr barrier );

    /** @return the current swap barrier. */
    SwapBarrierConstPtr getSwapBarrier() const { return _swapBarrier; }

    /**
     * Add a new input frame for this compound.
     *
     * @param frame the input frame.
     */
    EQSERVER_API void addInputFrame( Frame* frame );

    /** @return the vector of input frames. */
    const Frames& getInputFrames() const {return _inputFrames; }

    /**
     * Add a new output frame for this compound.
     *
     * @param frame the output frame.
     */
    EQSERVER_API void addOutputFrame( Frame* frame );

    /** @return the vector of output frames. */
    const Frames& getOutputFrames() const { return _outputFrames; }

    /**
     * Add a new input tile queue for this compound.
     *
     * @param tileQueue the input tile queue.
     */
    EQSERVER_API void addInputTileQueue( TileQueue* tileQueue );

    /**
     * Remove an input tile queue from this compound.
     *
     * @param tileQueue the input tile queue.
     */
    EQSERVER_API void removeInputTileQueue( TileQueue* tileQueue );

    /** @return the vector of input tile queues. */
    const TileQueues& getInputTileQueues() const { return _inputTileQueues;}

    /**
     * Add a new output tile queue for this compound.
     *
     * @param queue the output tile queue.
     */
    EQSERVER_API void addOutputTileQueue( TileQueue* queue );

    /**
     * Remove an output tile queue from this compound.
     *
     * @param tileQueue the output tile queue.
     */
    EQSERVER_API void removeOutputTileQueue( TileQueue* tileQueue );

    /** @return the vector of output tile queues. */
    const TileQueues& getOutputTileQueues() const { return _outputTileQueues; }

    /** @return true if the compound is a tile compound. */
    bool hasTiles() const
        { return !_outputTileQueues.empty() || !_inputTileQueues.empty(); }
    //@}

    /**
     * @name Inherit data access needed during channel update.
     *
     * Inherit data are the actual, as opposed to configured, attributes and
     * data used by the compound. The inherit data is updated at the
     * beginning of each update().
     */
    //@{
    RenderContext setupRenderContext( Eye eye ) const;
    uint32_t getInheritBuffers() const { return _inherit.buffers; }
    const PixelViewport& getInheritPixelViewport() const { return _inherit.pvp;}
    const Range& getInheritRange()   const { return _inherit.range; }
    const Pixel& getInheritPixel()   const { return _inherit.pixel; }
    const SubPixel& getInheritSubPixel() const { return _inherit.subPixel; }
    const Zoom& getInheritZoom()     const { return _inherit.zoom; }
    uint32_t getInheritPeriod()          const { return _inherit.period; }
    uint32_t getInheritPhase()           const { return _inherit.phase; }
    float getInheritMaxFPS()             const { return _inherit.maxFPS; }
    int32_t getInheritIAttribute( const IAttribute attr ) const
        { return _inherit.iAttributes[attr]; }
    uint32_t getInheritTasks()           const { return _inherit.tasks; }
    uint32_t getInheritEyes()            const { return _inherit.eyes; }
    Channel* getInheritChannel()               { return _inherit.channel; }
    const Channel* getInheritChannel()   const { return _inherit.channel; }

    /** @return true if the task is set, false if not. */
    bool testInheritTask( const fabric::Task task ) const
        { return (_inherit.tasks & task); }

    /** Delete an inherit task, if it was set. */
    void unsetInheritTask( const fabric::Task task ) { _inherit.tasks &= ~task;}

    /** @return true if the eye pass is actived, false if not. */
    bool testInheritEye( const Eye eye ) const { return (_inherit.eyes & eye); }
    //@}

    /** @name Frustum Operations */
    //@{
    /**
     * Set the compound's frustum using a wall description.
     *
     * @param wall the wall description.
     */
    EQSERVER_API void setWall( const Wall& wall );

    /** @return the last specified wall description. */
    const Wall& getWall() const { return _frustum.getWall(); }

    /**
     * Set the compound's frustum using a projection description
     *
     * @param projection the projection description.
     */
    EQSERVER_API void setProjection( const Projection& projection );

    /** @return the last specified projection description. */
    const Projection& getProjection() const
    { return _frustum.getProjection(); }

    /** @return the type of the latest specified frustum. */
    Frustum::Type getFrustumType() const
    { return _frustum.getCurrentType(); }

    /** @return the frustum of this compound. */
    Frustum& getFrustum() { return _frustum; }

    /** @return the frustum of this compound. */
    const Frustum& getFrustum() const { return _frustum; }

    /** Update the frustum from the view or segment. */
    void updateFrustum( const Vector3f& eye, const float ratio );

    /** compute the frustum for a given viewport */
    void computeTileFrustum( Frustumf& frustum, const fabric::Eye eye,
                             Viewport vp, bool ortho ) const;

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
    EQSERVER_API VisitorResult accept( CompoundVisitor& visitor ) const;
    /** Non-const version of accept(). */
    EQSERVER_API VisitorResult accept( CompoundVisitor& visitor );

    /** @internal Activate the given eyes for the the compound tree. */
    void activate( const uint32_t eyes );

    /** @internal Deactivate the given eyes for the the compound tree. */
    void deactivate( const uint32_t eyes );

    /**
     * @return if the compound is activated for selected eye
     and current (DPlex).
     *
     * @param eye eye which will be tested.
     */
    bool isInheritActive( const Eye eye ) const;

    /** @return true if the compound is activated for any later eye pass. */
    bool isLastInheritEye( const Eye eye ) const;

    /**
     * @return true if the compound is active and the compound's channel is
     *         running.
     */
    bool isActive() const;

    /** Initialize this compound. */
    void init();

    /** Exit this compound. */
    void exit();

    /** Initialize the default tasks of this compound. */
    void updateInheritTasks();

    /** Register all distributed objects (frames) */
    void register_();

    /** Deregister all distributed objects (frames) */
    void deregister();

    /** Back up all relevant compound data. */
    void backup();

    /** Restore all relevant compound data. */
    void restore();

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
    static const std::string&  getIAttributeString( const IAttribute attr );
    //@}

    typedef stde::hash_map<std::string, co::Barrier*> BarrierMap;
    typedef BarrierMap::const_iterator BarrierMapCIter;

    typedef stde::hash_map<std::string, Frame*> FrameMap;
    typedef FrameMap::const_iterator FrameMapCIter;

    typedef stde::hash_map<std::string, TileQueue*>   TileQueueMap;

private:
    //-------------------- Members --------------------
    std::string _name;

    /**
     * The config the compound is attached to, only set on root
     * compounds.
     */
    friend class Config;
    Config* const _config;

    Compound* const _parent;
    Compounds _children;

    /** Percentage the resource should be used. */
    float _usage;

    /** Unique identifier for channel tasks. */
    uint32_t _taskID;

    struct Data
    {
        Data();

        Channel*          channel;
        Viewport          vp;
        PixelViewport     pvp;
        Vector4i          overdraw;
        Range             range;
        Pixel             pixel;
        SubPixel          subPixel;
        FrustumData       frustumData;
        Zoom              zoom;
        uint32_t          buffers;
        uint32_t          eyes;
        uint32_t          tasks;
        uint32_t          period;
        uint32_t          phase;
        int32_t           iAttributes[IATTR_ALL];
        float             maxFPS;

        // compound activation per eye
        uint32_t active[ fabric::NUM_EYES ];

        union // placeholder for binary-compatible changes
        {
            char dummy[16];
        };
    };

    Data _data;
    Data _backup;
    Data _inherit;

    /** The frustum description of this compound. */
    Frustum _frustum;

    typedef std::vector< CompoundListener* > CompoundListeners;
    CompoundListeners _listeners;

    Equalizers _equalizers;

    SwapBarrierPtr _swapBarrier;

    Frames _inputFrames;
    Frames _outputFrames;

    TileQueues _inputTileQueues;
    TileQueues _outputTileQueues;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    LB_TS_VAR( _serverThread );

    //-------------------- Methods --------------------
    void _addChild( Compound* child );
    bool _removeChild( Compound* child );

    void _updateOverdraw( Wall& wall );
    void _updateInheritRoot();
    void _updateInheritNode();
    void _updateInheritPVP();
    void _updateInheritOverdraw();
    void _updateInheritStereo();
    void _updateInheritActive( const uint32_t frameNumber );

    void _setDefaultFrameName( Frame* frame );
    void _setDefaultTileQueueName( TileQueue* tileQueue );

    void _fireChildAdded( Compound* child );
    void _fireChildRemove( Compound* child );

    void _computeFrustum( RenderContext& context ) const;
    void _computePerspective( RenderContext& context,
                              const Vector3f& eye ) const;
    void _computeOrtho( RenderContext& context, const Vector3f& eye ) const;
    Vector3f _getEyePosition( const fabric::Eye eye ) const;
    const Matrix4f& _getInverseHeadMatrix() const;
    void _computeFrustumCorners( Frustumf& frustum,
                                 const FrustumData& frustumData, const Vector3f& eye,
                                 const bool ortho, const Viewport* const vp = 0 ) const;
};

std::ostream& operator << ( std::ostream& os, const Compound& compound );
}
}
#endif // EQSERVER_COMPOUND_H
