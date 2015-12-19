
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "pipe.h"

// must be included before any header defining Bool
#ifdef EQ_QT_USED
#  include "qt/window.h"
#  include <QThread>
#endif

#include "channel.h"
#include "client.h"
#include "config.h"
#include "exception.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "pipeStatistics.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include "messagePump.h"
#include "systemPipe.h"

#include "computeContext.h"
#ifdef EQUALIZER_USE_CUDA
#  include "cudaContext.h"
#endif

#include <eq/fabric/commands.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/leafVisitor.h>
#include <eq/fabric/task.h>

#include <co/global.h>
#include <co/objectICommand.h>
#include <co/queueSlave.h>
#include <co/worker.h>
#include <boost/lexical_cast.hpp>
#include <sstream>

#ifdef EQUALIZER_USE_HWLOC_GL
#  include <hwloc.h>
#  include <hwloc/gl.h>
#endif

#ifdef EQUALIZER_USE_QT5WIDGETS
#  include <QGuiApplication>
#  include <QRegularExpression>
#endif

namespace eq
{
/** @cond IGNORE */
typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;
typedef co::CommandFunc<Pipe> PipeFunc;
/** @endcond */

namespace
{
enum State
{
    STATE_MAPPED,
    STATE_INITIALIZING,
    STATE_RUNNING,
    STATE_STOPPING, // must come after running
    STATE_STOPPED, // must come after running
    STATE_FAILED
};

typedef stde::hash_map< uint128_t, Frame* > FrameHash;
typedef stde::hash_map< uint128_t, FrameDataPtr > FrameDataHash;
typedef stde::hash_map< uint128_t, View* > ViewHash;
typedef stde::hash_map< uint128_t, co::QueueSlave* > QueueHash;
typedef FrameHash::const_iterator FrameHashCIter;
typedef FrameDataHash::const_iterator FrameDataHashCIter;
typedef ViewHash::const_iterator ViewHashCIter;
typedef ViewHash::iterator ViewHashIter;
typedef QueueHash::const_iterator QueueHashCIter;
}

namespace detail
{

class RenderThread : public eq::Worker
{
public:
    explicit RenderThread( eq::Pipe* pipe )
        : eq::Worker( co::Global::getCommandQueueLimit( ))
        , _pipe( pipe )
    {}

protected:
    bool init() override
    {
        setName( std::string( "Draw" ) +
              boost::lexical_cast< std::string >( _pipe->getPath().pipeIndex ));
        return true;
    }

    void run() override;
    bool stopRunning() override { return !_pipe; }

private:
    eq::Pipe* _pipe;
    friend class eq::Pipe;
};


/** Asynchronous, per-pipe readback thread. */
class TransferThread : public co::Worker
{
public:
    explicit TransferThread( const uint32_t index )
        : co::Worker( co::Global::getCommandQueueLimit( ))
        , _index( index )
        , _qThread( nullptr )
        , _stop( false )
    {}

    bool init() override
    {
        if( !co::Worker::init( ))
            return false;
        setName( std::string( "Tfer" ) +
                 boost::lexical_cast< std::string >( _index ));
#ifdef EQ_QT_USED
        _qThread = QThread::currentThread();
#endif
        return true;
    }

    bool stopRunning() override { return _stop; }
    void postStop() { _stop = true; }

    QThread* getQThread() { return _qThread; }

private:
    uint32_t _index;
    QThread* _qThread;
    bool _stop; // thread will exit if this is true
};

class Pipe
{
public:
    explicit Pipe( const uint32_t index )
        : systemPipe( 0 )
#ifdef AGL
        , windowSystem( "AGL" )
#elif GLX
        , windowSystem( "GLX" )
#elif WGL
        , windowSystem( "WGL" )
#elif EQ_QT_USED
        , windowSystem( "Qt" )
#endif
        , state( STATE_STOPPED )
        , currentFrame( 0 )
        , frameTime( 0 )
        , thread( 0 )
        , transferThread( index )
        , computeContext( 0 )
    {}

    ~Pipe()
    {
        delete thread;
        thread = 0;
    }

    /** Window-system specific functions class */
    SystemPipe* systemPipe;

    /** The current window system. */
    WindowSystem windowSystem;

    /** The configInit/configExit state. */
    lunchbox::Monitor< State > state;

    /** The last started frame. */
    uint32_t currentFrame;

    /** The number of the last finished frame. */
    lunchbox::Monitor< uint32_t > finishedFrame;

    /** The number of the last locally unlocked frame. */
    lunchbox::Monitor< uint32_t > unlockedFrame;

    /** The running per-frame statistic clocks. */
    std::deque< int64_t > frameTimes;
    lunchbox::Lock frameTimeMutex;

    /** The base time for the currently active frame. */
    int64_t frameTime;

    /** All assembly frames used by the pipe during rendering. */
    FrameHash frames;

    /** All output frame datas used by the pipe during rendering. */
    FrameDataHash outputFrameDatas;

    /** All input frame datas used by the pipe during rendering. */
    FrameDataHash inputFrameDatas;

    /** All views used by the pipe's channels during rendering. */
    ViewHash views;

    /** All queues used by the pipe's channels during rendering. */
    QueueHash queues;

    /** The pipe thread. */
    RenderThread* thread;

    detail::TransferThread transferThread;

    /** GPU Computing context */
    ComputeContext *computeContext;
};

void RenderThread::run()
{
    LB_TS_THREAD( _pipe->_pipeThread );
    LBDEBUG << "Entered pipe thread" << std::endl;

    eq::Pipe* pipe = _pipe; // _pipe gets cleared on exit
    pipe->_impl->state.waitEQ( STATE_MAPPED );
    pipe->_impl->windowSystem = pipe->selectWindowSystem();
    pipe->_setupCommandQueue();
    pipe->_setupAffinity();

    Worker::run();

    pipe->_exitCommandQueue();
}
}

Pipe::Pipe( Node* parent )
        : Super( parent )
        , _impl( new detail::Pipe( getPath().pipeIndex ))
{
}

Pipe::~Pipe()
{
    LBASSERT( getWindows().empty( ));
    delete _impl;
}

Config* Pipe::getConfig()
{
    Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getConfig() : 0);
}

const Config* Pipe::getConfig() const
{
    const Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getConfig() : 0);
}

ClientPtr Pipe::getClient()
{
    Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getClient() : 0);
}

ServerPtr Pipe::getServer()
{
    Node* node = getNode();
    LBASSERT( node );
    return ( node ? node->getServer() : 0);
}

void Pipe::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* queue = getPipeThreadQueue();
    co::CommandQueue* transferQ = getTransferThreadQueue();

    registerCommand( fabric::CMD_PIPE_CONFIG_INIT,
                     PipeFunc( this, &Pipe::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_PIPE_CONFIG_EXIT,
                     PipeFunc( this, &Pipe::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_PIPE_CREATE_WINDOW,
                     PipeFunc( this, &Pipe::_cmdCreateWindow ), queue );
    registerCommand( fabric::CMD_PIPE_DESTROY_WINDOW,
                     PipeFunc( this, &Pipe::_cmdDestroyWindow ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_START,
                     PipeFunc( this, &Pipe::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_FINISH,
                     PipeFunc( this, &Pipe::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_DRAW_FINISH,
                     PipeFunc( this, &Pipe::_cmdFrameDrawFinish ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_START_CLOCK,
                     PipeFunc( this, &Pipe::_cmdFrameStartClock ), 0 );
    registerCommand( fabric::CMD_PIPE_EXIT_THREAD,
                     PipeFunc( this, &Pipe::_cmdExitThread ), queue );
    registerCommand( fabric::CMD_PIPE_DETACH_VIEW,
                     PipeFunc( this, &Pipe::_cmdDetachView ), queue );
    registerCommand( fabric::CMD_PIPE_EXIT_TRANSFER_THREAD,
                     PipeFunc( this, &Pipe::_cmdExitTransferThread ),
                     transferQ );
}

void Pipe::setDirty( const uint64_t bits )
{
    // jump over fabric setDirty to avoid dirty'ing node pipes list
    // pipes are individually synced in frame finish for thread-safety
    Object::setDirty( bits );
}

bool Pipe::isWindowSystemAvailable( const std::string& name ) const
{
    bool available = false;
    if( name != "Qt" )
    {
#ifdef AGL
        available = name == "AGL";
#elif GLX
        available = name == "GLX";
#elif WGL
        available = name == "WGL";
#endif
    }

#ifdef EQ_QT_USED
    if( name != "Qt" )
        return available;

    // Qt can only use ports that refer to QScreens available to the
    // QApplication. In Windows and Mac all physical displays are
    // queriable using platform dependent functions (no idea about virtual
    // displays like Xming or XQuartz), so we will assume that if Equalizer
    // was built with Qt support, then all devices can be used by Qt.
    // (How to choose the right screen for a given device it's a different
    // story)
#  ifdef AGL
    available = true;
#  elif WGL
    available = true;
#  else
    // For X it's different. Qt can only use screens that are part of the
    // display server referred by the DISPLAY environmental variable (in
    // particular its value at the moment of the creation of the QApplication)
#    ifdef __APPLE__
    // In MAC this is simpler, because there can only be one display server.
    return true;
#    else
    // There's no way to infer the X server number from the Qt API except
    // QScreen names, but only under certain conditions. In regular desktop
    // usage QScreen::name gives a name related to what xrandr prints, so it's
    // not usable. The names seems to follow the X naming convention
    // (e.g. host:0.0) when XRANDR is not available (e.g. Xnest) or doesn't
    // provide information about where the displays are connected (e.g.
    // headless or Xvnc). These are corner cases so we cannot rely on QScreen.
    // Therefore, we will infer which port is Qt using directly from DISPLAY.

    // Is the port is undefined that means that the default server must
    // be used. This should match the Qt display server.
    if( getPort() == LB_UNDEFINED_UINT32 )
        return true;

    QGuiApplication* app =
        dynamic_cast< QGuiApplication* >( QCoreApplication::instance( ));
    if( !app || !app->primaryScreen() )
        return false; // Qt won't be able to access anything anyway

    QRegularExpression regex( "^[a-z]*\\:([0-9]+)(\\.[0-9]+)?$" );
    QRegularExpressionMatch match =
        regex.match( getenv( "DISPLAY" ) ? getenv( "DISPLAY" ) : "" );
    available = match.captured( 1 ) == QString::number( getPort(), 10 );
#    endif
#  endif
#endif

    return available;
}

WindowSystem Pipe::selectWindowSystem() const
{
#ifdef AGL
    return WindowSystem( "AGL" );
#elif GLX
    return WindowSystem( "GLX" );
#elif WGL
    return WindowSystem( "WGL" );
#elif EQ_QT_USED
    if( !isWindowSystemAvailable( "Qt" ))
    {
        // Throwing because there's no reasonable alternative.
        std::stringstream msg;
        msg << "Cannot choose windowing system for pipe at port " << getPort()
            << ". Qt was set as the default, but it cannot be used. In X"
               " based systems this means that the value of DISPLAY taken by"
               " Qt refers to a different display server." << std::endl;
        LBTHROW( std::runtime_error( msg.str( )));
    }
    return WindowSystem( "Qt" );
#endif
}

void Pipe::_setupCommandQueue()
{
    LBDEBUG << "Set up pipe message pump for " << _impl->windowSystem
           << std::endl;

    Config* config = getConfig();
    config->setupMessagePump( this );

    if( !_impl->thread ) // Non-threaded pipes have no pipe thread message pump
        return;

    CommandQueue* queue = _impl->thread->getWorkerQueue();
    LBASSERT( queue );
    LBASSERT( !queue->getMessagePump( ));

    Global::enterCarbon();
    MessagePump* pump = createMessagePump();
    if( pump )
        pump->dispatchAll(); // initializes _impl->receiverQueue

    queue->setMessagePump( pump );
    Global::leaveCarbon();
}

int32_t Pipe::_getAutoAffinity() const
{
#ifdef EQUALIZER_USE_HWLOC_GL
    uint32_t port = getPort();
    uint32_t device = getDevice();

    if( port == LB_UNDEFINED_UINT32 && device == LB_UNDEFINED_UINT32 )
        return lunchbox::Thread::NONE;

    if( port == LB_UNDEFINED_UINT32 )
        port = 0;
    if( device == LB_UNDEFINED_UINT32 )
        device = 0;

    hwloc_topology_t topology;
    if( hwloc_topology_init( &topology ) < 0 )
    {
        LBINFO << "Automatic pipe thread placement failed: "
               << "hwloc_topology_init() failed" << std::endl;
        return lunchbox::Thread::NONE;
    }

    // Load I/O devices, bridges and their relevant info
    const unsigned long loading_flags = HWLOC_TOPOLOGY_FLAG_IO_BRIDGES |
                                        HWLOC_TOPOLOGY_FLAG_IO_DEVICES;
    if( hwloc_topology_set_flags( topology, loading_flags ) < 0 )
    {
        LBINFO << "Automatic pipe thread placement failed: "
               << "hwloc_topology_set_flags() failed" << std::endl;
        hwloc_topology_destroy( topology );
        return lunchbox::Thread::NONE;
    }

    if( hwloc_topology_load( topology ) < 0 )
    {
        LBINFO << "Automatic pipe thread placement failed: "
               << "hwloc_topology_load() failed" << std::endl;
        hwloc_topology_destroy( topology );
        return lunchbox::Thread::NONE;
    }

    const hwloc_obj_t osdev =
        hwloc_gl_get_display_osdev_by_port_device( topology,
                                                   int( port ), int( device ));
    if( !osdev )
    {
        LBINFO << "Automatic pipe thread placement failed: GPU not found"
               << std::endl;
        hwloc_topology_destroy( topology );
        return lunchbox::Thread::NONE;
    }

    const hwloc_obj_t pcidev = osdev->parent;
    const hwloc_obj_t parent = hwloc_get_non_io_ancestor_obj( topology, pcidev);
    const int numCpus =
        hwloc_get_nbobjs_inside_cpuset_by_type( topology, parent->cpuset,
                                                HWLOC_OBJ_SOCKET );
    if( numCpus != 1 )
    {
        LBINFO << "Automatic pipe thread placement failed: GPU attached to "
               << numCpus << " processors?" << std::endl;
        hwloc_topology_destroy( topology );
        return lunchbox::Thread::NONE;
    }

    const hwloc_obj_t cpuObj =
        hwloc_get_obj_inside_cpuset_by_type( topology, parent->cpuset,
                                             HWLOC_OBJ_SOCKET, 0 );
    if( cpuObj == 0 )
    {
        LBINFO << "Automatic pipe thread placement failed: "
               << "hwloc_get_obj_inside_cpuset_by_type() failed" << std::endl;
        hwloc_topology_destroy( topology );
        return lunchbox::Thread::NONE;
    }

    const int cpuIndex = cpuObj->logical_index;
    hwloc_topology_destroy( topology );
    return cpuIndex + lunchbox::Thread::SOCKET;
#else
    LBDEBUG << "Automatic thread placement not supported, no hwloc GL support"
            << std::endl;
#endif
    return lunchbox::Thread::NONE;
}

void Pipe::_setupAffinity()
{
    const int32_t affinity = getIAttribute( IATTR_HINT_AFFINITY );
    switch( affinity )
    {
        case AUTO:
            lunchbox::Thread::setAffinity( _getAutoAffinity( ));
            break;

        case OFF:
        default:
            lunchbox::Thread::setAffinity( affinity );
            break;
    }
}

void Pipe::_exitCommandQueue()
{
    // Non-threaded pipes have no pipe thread message pump
    if( !_impl->thread )
        return;

    CommandQueue* queue = _impl->thread->getWorkerQueue();
    LBASSERT( queue );

    MessagePump* pump = queue->getMessagePump();
    queue->setMessagePump( 0 );
    delete pump;
}

MessagePump* Pipe::createMessagePump()
{
    return _impl->windowSystem.createMessagePump();
}

MessagePump* Pipe::getMessagePump()
{
    LB_TS_THREAD( _pipeThread );
    if( !_impl->thread )
        return 0;

    CommandQueue* queue = _impl->thread->getWorkerQueue();
    return queue->getMessagePump();
}

co::CommandQueue* Pipe::getPipeThreadQueue()
{
    if( _impl->thread )
        return _impl->thread->getWorkerQueue();

    return getNode()->getMainThreadQueue();
}

co::CommandQueue* Pipe::getTransferThreadQueue()
{
    return _impl->transferThread.getWorkerQueue();
}

co::CommandQueue* Pipe::getMainThreadQueue()
{
    return getServer()->getMainThreadQueue();
}

co::CommandQueue* Pipe::getCommandThreadQueue()
{
    return getServer()->getCommandThreadQueue();
}

Frame* Pipe::getFrame( const co::ObjectVersion& frameVersion, const Eye eye,
                       const bool isOutput )
{
    LB_TS_THREAD( _pipeThread );
    Frame* frame = _impl->frames[ frameVersion.identifier ];

    if( !frame )
    {
        ClientPtr client = getClient();
        frame = new Frame();

        LBCHECK( client->mapObject( frame, frameVersion ));
        _impl->frames[ frameVersion.identifier ] = frame;
    }
    else
        frame->sync( frameVersion.version );

    const co::ObjectVersion& dataVersion = frame->getDataVersion( eye );
    LBLOG( LOG_ASSEMBLY ) << "Use " << dataVersion << std::endl;

    FrameDataPtr frameData = getNode()->getFrameData( dataVersion );
    LBASSERT( frameData );

    if( isOutput )
    {
        if( !frameData->isAttached() )
        {
            ClientPtr client = getClient();
            LBCHECK( client->mapObject( frameData.get(), dataVersion ));
        }
        else if( frameData->getVersion() < dataVersion.version )
            frameData->sync( dataVersion.version );

        _impl->outputFrameDatas[ dataVersion.identifier ] = frameData;
    }
    else
        _impl->inputFrameDatas[ dataVersion.identifier ] = frameData;

    frame->setFrameData( frameData );
    return frame;
}

void Pipe::flushFrames( util::ObjectManager& om )
{
    LB_TS_THREAD( _pipeThread );
    ClientPtr client = getClient();
    for( FrameHashCIter i = _impl->frames.begin(); i !=_impl->frames.end(); ++i)
    {
        Frame* frame = i->second;
        frame->setFrameData( 0 ); // datas are flushed below
        client->unmapObject( frame );
        delete frame;
    }
    _impl->frames.clear();

    for( FrameDataHashCIter i = _impl->inputFrameDatas.begin();
         i != _impl->inputFrameDatas.end(); ++i )
    {
        FrameDataPtr data = i->second;
        data->deleteGLObjects( om );
    }
    _impl->inputFrameDatas.clear();

    for( FrameDataHashCIter i = _impl->outputFrameDatas.begin();
         i != _impl->outputFrameDatas.end(); ++i )
    {
        FrameDataPtr data = i->second;
        data->resetPlugins();
        data->deleteGLObjects( om );
        client->unmapObject( data.get( ));
        getNode()->releaseFrameData( data );
    }
    _impl->outputFrameDatas.clear();
}

co::QueueSlave* Pipe::getQueue( const uint128_t& queueID )
{
    LB_TS_THREAD( _pipeThread );
    if( queueID == 0 )
        return 0;

    co::QueueSlave* queue = _impl->queues[ queueID ];
    if( !queue )
    {
        queue = new co::QueueSlave;
        ClientPtr client = getClient();
        LBCHECK( client->mapObject( queue, queueID ));

        _impl->queues[ queueID ] = queue;
    }

    return queue;
}

void Pipe::_flushQueues()
{
    LB_TS_THREAD( _pipeThread );
    ClientPtr client = getClient();

    for( QueueHashCIter i = _impl->queues.begin(); i !=_impl->queues.end(); ++i)
    {
        co::QueueSlave* queue = i->second;
        client->unmapObject( queue );
        delete queue;
    }
    _impl->queues.clear();
}

const View* Pipe::getView( const co::ObjectVersion& viewVersion ) const
{
    // Yie-ha: we want to have a const-interface to get a view on the render
    //         clients, but view mapping is by definition non-const.
    return const_cast< Pipe* >( this )->getView( viewVersion );
}

View* Pipe::getView( const co::ObjectVersion& viewVersion )
{
    LB_TS_THREAD( _pipeThread );
    if( viewVersion.identifier == 0 )
        return 0;

    View* view = _impl->views[ viewVersion.identifier ];
    if( !view )
    {
        NodeFactory* nodeFactory = Global::getNodeFactory();
        view = nodeFactory->createView( 0 );
        LBASSERT( view );
        view->_pipe = this;
        ClientPtr client = getClient();
        LBCHECK( client->mapObject( view, viewVersion ));

        _impl->views[ viewVersion.identifier ] = view;
    }

    view->sync( viewVersion.version );
    return view;
}

void Pipe::_releaseViews()
{
    LB_TS_THREAD( _pipeThread );
    for( bool changed = true; changed; )
    {
        changed = false;
        for( ViewHashIter i =_impl->views.begin(); i !=_impl->views.end(); ++i )
        {
            View* view = i->second;
            view->commit();
            if( view->getVersion() + 20 > view->getHeadVersion( ))
                continue;

            // release unused view to avoid memory leaks due to deltas piling up
            view->_pipe = 0;

            ClientPtr client = getClient();
            client->unmapObject( view );
            _impl->views.erase( i );

            NodeFactory* nodeFactory = Global::getNodeFactory();
            nodeFactory->releaseView( view );

            changed = true;
            break;
        }
    }
}

void Pipe::_flushViews()
{
    LB_TS_THREAD( _pipeThread );
    NodeFactory* nodeFactory = Global::getNodeFactory();
    ClientPtr client = getClient();

    for( ViewHashCIter i = _impl->views.begin(); i != _impl->views.end(); ++i )
    {
        View* view = i->second;

        client->unmapObject( view );
        view->_pipe = 0;
        nodeFactory->releaseView( view );
    }
    _impl->views.clear();
}

void Pipe::startThread()
{
    _impl->thread = new detail::RenderThread( this );
    _impl->thread->start();
}

void Pipe::exitThread()
{
    _stopTransferThread();

    if( !_impl->thread )
        return;

    send( getLocalNode(), fabric::CMD_PIPE_EXIT_THREAD );

    _impl->thread->join();
    delete _impl->thread;
    _impl->thread = 0;
}

void Pipe::cancelThread()
{
    _stopTransferThread();

    if( !_impl->thread )
        return;

    // local command dispatching
    co::ObjectOCommand( this, getLocalNode(), fabric::CMD_PIPE_EXIT_THREAD,
                        co::COMMANDTYPE_OBJECT, getID(), CO_INSTANCE_ALL );
}

void Pipe::waitExited() const
{
    _impl->state.waitGE( STATE_STOPPED );
}

bool Pipe::isRunning() const
{
    return (_impl->state == STATE_RUNNING);
}

bool Pipe::isStopped() const
{
    return (_impl->state == STATE_STOPPED);
}

void Pipe::notifyMapped()
{
    LBASSERT( _impl->state == STATE_STOPPED );
    _impl->state = STATE_MAPPED;
}

namespace
{
class WaitFinishedVisitor : public PipeVisitor
{
public:
    WaitFinishedVisitor( const uint32_t frame, MessagePump* pump )
        : _frame( frame ), _pump( pump ) {}

    virtual VisitorResult visit( Channel* channel )
    {
        while( !channel->waitFrameFinished( _frame, 100 ))
        {
            // process potential pending Qt slots
            if( _pump )
                _pump->dispatchAll();
        }

        return TRAVERSE_CONTINUE;
    }

private:
    const uint32_t _frame;
    MessagePump* _pump;
};
}

void Pipe::waitFrameFinished( const uint32_t frameNumber )
{
    MessagePump* pump = getConfig()->getMessagePump();
    while( !_impl->finishedFrame.timedWaitGE( frameNumber, 100 ))
    {
        // process potential pending Qt slots
        if( pump )
            pump->dispatchAll();
    }

    WaitFinishedVisitor waiter( frameNumber, pump );
    accept( waiter );
}

void Pipe::waitFrameLocal( const uint32_t frameNumber ) const
{
    _impl->unlockedFrame.waitGE( frameNumber );
}

uint32_t Pipe::getCurrentFrame() const
{
    LB_TS_THREAD( _pipeThread );
    return _impl->currentFrame;
}

uint32_t Pipe::getFinishedFrame() const
{
    return _impl->finishedFrame.get();
}

WindowSystem Pipe::getWindowSystem() const
{
    return _impl->windowSystem;
}

EventOCommand Pipe::sendError( const uint32_t error )
{
    return getConfig()->sendError( Event::PIPE_ERROR, Error( error, getID( )));
}

bool Pipe::processEvent( const Event& event )
{
    ConfigEvent configEvent( event );
    getConfig()->sendEvent( configEvent );
    return true;
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
bool Pipe::configInit( const uint128_t& initID )
{
    LB_TS_THREAD( _pipeThread );

    LBASSERT( !_impl->systemPipe );

    if ( !configInitSystemPipe( initID ))
        return false;

    // -------------------------------------------------------------------------
    LBASSERT(!_impl->computeContext);

    // for now we only support CUDA
#ifdef EQUALIZER_USE_CUDA
    if( getIAttribute( IATTR_HINT_CUDA_GL_INTEROP ) == eq::ON )
    {
        LBDEBUG << "Initializing CUDAContext" << std::endl;
        ComputeContext* computeCtx = new CUDAContext( this );

        if( !computeCtx->configInit() )
        {
            LBWARN << "GPU Computing context initialization failed "
                   << std::endl;
            delete computeCtx;
            return false;
        }
        setComputeContext( computeCtx );
    }
#endif

    return true;
}

bool Pipe::configInitSystemPipe( const uint128_t& )
{
    SystemPipe* systemPipe = _impl->windowSystem.createPipe( this );
    LBASSERT( systemPipe );

    if( !systemPipe->configInit( ))
    {
        LBERROR << "System pipe context initialization failed" << std::endl;
        delete systemPipe;
        return false;
    }

    setSystemPipe( systemPipe );
    return true;
}

bool Pipe::configExit()
{
    LB_TS_THREAD( _pipeThread );

    if( _impl->computeContext )
    {
        _impl->computeContext->configExit();
        delete _impl->computeContext;
        _impl->computeContext = 0;
    }

    if( _impl->systemPipe )
    {
        _impl->systemPipe->configExit( );
        delete _impl->systemPipe;
        _impl->systemPipe = 0;
    }
    return true;
}


void Pipe::frameStart( const uint128_t&, const uint32_t frameNumber )
{
    LB_TS_THREAD( _pipeThread );

    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release immediately
            releaseFrameLocal( frameNumber );
            break;

        case DRAW_SYNC:  // Sync, release in frameDrawFinish
        case LOCAL_SYNC: // Sync, release in frameFinish
            node->waitFrameStarted( frameNumber );
            break;

        default:
            LBUNIMPLEMENTED;
    }

    startFrame( frameNumber );
}

void Pipe::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // released in frameStart
            break;

        case DRAW_SYNC:  // release
            releaseFrameLocal( frameNumber );
            break;

        case LOCAL_SYNC: // release in frameFinish
            break;

        default:
            LBUNIMPLEMENTED;
    }
}

void Pipe::frameFinish( const uint128_t&, const uint32_t frameNumber )
{
    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // released in frameStart
            break;

        case DRAW_SYNC:  // released in frameDrawFinish
            break;

        case LOCAL_SYNC: // release
            releaseFrameLocal( frameNumber );
            break;

        default:
            LBUNIMPLEMENTED;
    }

    // Global release
    releaseFrame( frameNumber );
}

void Pipe::startFrame( const uint32_t frameNumber )
{
    LB_TS_THREAD( _pipeThread );
    _impl->currentFrame = frameNumber;
    LBLOG( LOG_TASKS ) << "---- Started Frame ---- "<< frameNumber << std::endl;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{
    LB_TS_THREAD( _pipeThread );
    _impl->finishedFrame = frameNumber;
    LBLOG( LOG_TASKS ) << "---- Finished Frame --- "<< frameNumber << std::endl;
}

void Pipe::releaseFrameLocal( const uint32_t frameNumber )
{
    LB_TS_THREAD( _pipeThread );
    LBASSERTINFO( _impl->unlockedFrame + 1 == frameNumber,
                  _impl->unlockedFrame << ", " << frameNumber );

    _impl->unlockedFrame = frameNumber;
    LBLOG( LOG_TASKS ) << "---- Unlocked Frame --- "
                       << _impl->unlockedFrame.get() << std::endl;
}

bool Pipe::startTransferThread()
{
    if( _impl->transferThread.isRunning( ))
        return true;

    return _impl->transferThread.start();
}

QThread* Pipe::getTransferQThread()
{
    return _impl->transferThread.getQThread();
}

bool Pipe::hasTransferThread() const
{
    return _impl->transferThread.isRunning();
}

void Pipe::_stopTransferThread()
{
    if( _impl->transferThread.isStopped( ))
        return;

    send( getLocalNode(), fabric::CMD_PIPE_EXIT_TRANSFER_THREAD );
    _impl->transferThread.join();
}

void Pipe::setSystemPipe( SystemPipe* pipe )
{
    _impl->systemPipe = pipe;
}

SystemPipe* Pipe::getSystemPipe()
{
    return _impl->systemPipe;
}

const SystemPipe* Pipe::getSystemPipe() const
{
    return _impl->systemPipe;
}

void Pipe::setComputeContext( ComputeContext* ctx )
{
    _impl->computeContext = ctx;
}

const ComputeContext* Pipe::getComputeContext() const
{
    return _impl->computeContext;
}

ComputeContext* Pipe::getComputeContext()
{
    return _impl->computeContext;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Pipe::_cmdCreateWindow( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& windowID = command.read< uint128_t >();

    LBLOG( LOG_INIT ) << "Create window " << command << " id " << windowID
                      << std::endl;

    Window* window = Global::getNodeFactory()->createWindow( this );
    window->init(); // not in ctor, virtual method

    Config* config = getConfig();
    LBCHECK( config->mapObject( window, windowID ));

    return true;
}

bool Pipe::_cmdDestroyWindow( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_INIT ) << "Destroy window " << command << std::endl;

    Window* window = _findWindow( command.read< uint128_t >( ));
    LBASSERT( window );

    // re-set shared windows accordingly
    Window* newSharedWindow = 0;
    const Windows& windows = getWindows();
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        Window* candidate = *i;

        if( candidate == window )
            continue; // ignore

        if( candidate->getSharedContextWindow() == window )
        {
            if( newSharedWindow )
                candidate->setSharedContextWindow( newSharedWindow );
            else
            {
                newSharedWindow = candidate;
                newSharedWindow->setSharedContextWindow( candidate );
            }
        }

        LBASSERT( candidate->getSharedContextWindow() != window );
    }

    const bool stopped = window->isStopped();
    window->send( getServer(),
                  fabric::CMD_WINDOW_CONFIG_EXIT_REPLY ) << stopped;

    Config* config = getConfig();
    config->unmapObject( window );
    Global::getNodeFactory()->releaseWindow( window );

    return true;
}

bool Pipe::_cmdConfigInit( co::ICommand& cmd )
{
    LB_TS_THREAD( _pipeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& initID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_INIT ) << "Init pipe " << command << " init id " << initID
                      << " frame " << frameNumber << std::endl;

    if( !isThreaded( ))
    {
        _impl->windowSystem = selectWindowSystem();
        _setupCommandQueue();
    }

    Node* node = getNode();
    LBASSERT( node );
    node->waitInitialized();

    bool result = false;
    if( node->isRunning( ))
    {
        _impl->currentFrame  = frameNumber;
        _impl->finishedFrame = frameNumber;
        _impl->unlockedFrame = frameNumber;
        _impl->state = STATE_INITIALIZING;

        result = configInit( initID );

        if( result )
            _impl->state = STATE_RUNNING;
    }
    else
        sendError( ERROR_PIPE_NODE_NOTRUNNING );

    LBLOG( LOG_INIT ) << "TASK pipe config init reply result " << result
                      << std::endl;

    commit();
    send( command.getRemoteNode(), fabric::CMD_PIPE_CONFIG_INIT_REPLY )
            << result;
    return true;
}

bool Pipe::_cmdConfigExit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _pipeThread );
    LBLOG( LOG_INIT ) << "TASK pipe config exit " << command << std::endl;

    _impl->state = STATE_STOPPING; // needed in View::detach (from _flushViews)

    // send before node gets a chance to send its destroy command
    getNode()->send( getLocalNode(), fabric::CMD_NODE_DESTROY_PIPE ) << getID();

    // Flush views before exit since they are created after init
    // - application may need initialized pipe to exit
    // - configExit can't access views since all channels are gone already
    _flushViews();
    _flushQueues();
    _impl->state = configExit() ? STATE_STOPPED : STATE_FAILED;
    return true;
}

bool Pipe::_cmdExitThread( co::ICommand& )
{
    LBASSERT( _impl->thread );
    _impl->thread->_pipe = 0;
    return true;
}

bool Pipe::_cmdExitTransferThread( co::ICommand& )
{
    _impl->transferThread.postStop();
    return true;
}

bool Pipe::_cmdFrameStartClock( co::ICommand& )
{
    LBVERB << "start frame clock" << std::endl;
    _impl->frameTimeMutex.set();
    _impl->frameTimes.push_back( getConfig()->getTime( ));
    _impl->frameTimeMutex.unset();
    return true;
}

bool Pipe::_cmdFrameStart( co::ICommand& cmd )
{
    LB_TS_THREAD( _pipeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& version = command.read< uint128_t >();
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBVERB << "handle pipe frame start " << command << " frame " << frameNumber
           << " id " << frameID << std::endl;

    LBLOG( LOG_TASKS ) << "---- TASK start frame ---- frame " << frameNumber
                       << " id " << frameID << std::endl;
    sync( version );
    const int64_t lastFrameTime = _impl->frameTime;

    _impl->frameTimeMutex.set();
    LBASSERT( !_impl->frameTimes.empty( ));

    _impl->frameTime = _impl->frameTimes.front();
    _impl->frameTimes.pop_front();
    _impl->frameTimeMutex.unset();

    if( lastFrameTime > 0 )
    {
        PipeStatistics waitEvent( Statistic::PIPE_IDLE, this );
        waitEvent.event.data.statistic.idleTime =
            _impl->thread ? _impl->thread->getWorkerQueue()->resetWaitTime() :0;
        waitEvent.event.data.statistic.totalTime =
            LB_MAX( _impl->frameTime - lastFrameTime, 1 ); // avoid SIGFPE
    }

    LBASSERTINFO( _impl->currentFrame + 1 == frameNumber,
                  "current " <<_impl->currentFrame << " start " << frameNumber);

    frameStart( frameID, frameNumber );
    return true;
}

bool Pipe::_cmdFrameFinish( co::ICommand& cmd )
{
    LB_TS_THREAD( _pipeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "---- TASK finish frame --- " << command << " frame "
                       << frameNumber << " id " << frameID << std::endl;

    LBASSERTINFO( _impl->currentFrame >= frameNumber,
                  "current " <<_impl->currentFrame << " finish " <<frameNumber);

    frameFinish( frameID, frameNumber );

    LBASSERTINFO( _impl->finishedFrame >= frameNumber,
                  "Pipe::frameFinish() did not release frame " << frameNumber );

    if( _impl->unlockedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not locally unlocked, enforcing unlock"
               << std::endl << "    unlocked " << _impl->unlockedFrame.get()
               << " done " << frameNumber << std::endl;
        releaseFrameLocal( frameNumber );
    }

    if( _impl->finishedFrame < frameNumber )
    {
        LBWARN << "Finished frame was not released, enforcing unlock"
               << std::endl;
        releaseFrame( frameNumber );
    }

    _releaseViews();

    const uint128_t version = commit();
    if( version != co::VERSION_NONE )
        send( command.getRemoteNode(), fabric::CMD_OBJECT_SYNC );
    return true;
}

bool Pipe::_cmdFrameDrawFinish( co::ICommand& cmd )
{
    LB_TS_THREAD( _pipeThread );

    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK draw finish " << getName()
                       << " frame " << frameNumber << " id " << frameID
                       << std::endl;

    frameDrawFinish( frameID, frameNumber );
    return true;
}

bool Pipe::_cmdDetachView( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _pipeThread );

    ViewHash::iterator i = _impl->views.find( command.read< uint128_t >( ));
    if( i != _impl->views.end( ))
    {
        View* view = i->second;
        _impl->views.erase( i );

        NodeFactory* nodeFactory = Global::getNodeFactory();
        nodeFactory->releaseView( view );
    }
    return true;
}

}

#include <eq/fabric/pipe.ipp>
template class eq::fabric::Pipe< eq::Node, eq::Pipe, eq::Window,
                                 eq::PipeVisitor >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                const eq::Super& );
/** @endcond */
