
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010, Cedric Stalder <cedric.stalder@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "configEvent.h"

#include "view.h"
#include "modelAssigner.h"

#include <admin/addWindow.h>
#include <admin/removeWindow.h>

namespace eqPly
{

Config::Config( eq::ServerPtr parent )
        : eq::Config( parent )
        , _spinX( 5 )
        , _spinY( 5 )
        , _advance( 0 )
        , _currentCanvas( 0 )
        , _messageTime( 0 )
        , _redraw( true )
        , _useIdleAA( true )
        , _numFramesAA( 0 )
{
}

Config::~Config()
{
    for( ModelsCIter i = _models.begin(); i != _models.end(); ++i )
    {
        delete *i;
    }
    _models.clear();

    for( ModelDistsCIter i = _modelDist.begin(); i != _modelDist.end(); ++i )
    {
        EQASSERT( !(*i)->isAttached() );
        delete *i;
    }
    _modelDist.clear();
}

bool Config::init()
{
    if( !_animation.isValid( ))
        _animation.loadAnimation( _initData.getPathFilename( ));

    // init distributed objects
    if( !_initData.useColor( ))
        _frameData.setColorMode( COLOR_WHITE );

    _frameData.setRenderMode( _initData.getRenderMode( ));
    registerObject( &_frameData );
    _frameData.setAutoObsolete( getLatency( ));

    _initData.setFrameDataID( _frameData.getID( ));
    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
    {
        _deregisterData();
        return false;
    }

    _loadModels();
    _registerModels();

    // init tracker
    if( !_initData.getTrackerPort().empty( ))
    {
        if( !_tracker.init( _initData.getTrackerPort() ))
            EQWARN << "Failed to initialize tracker" << std::endl;
        else
        {
            // Set up position of tracking system wrt world space
            // Note: this depends on the physical installation.
            eq::Matrix4f m( eq::Matrix4f::IDENTITY );
            m.scale( 1.f, 1.f, -1.f );
            _tracker.setWorldToEmitter( m );

            m = eq::Matrix4f::IDENTITY;
            m.rotate_z( -M_PI_2 );
            _tracker.setSensorToObject( m );
            EQINFO << "Tracker initialized" << std::endl;
        }
    }

    const eq::Canvases& canvases = getCanvases();
    if( canvases.empty( ))
        _currentCanvas = 0;
    else
        _currentCanvas = canvases.front();

    _setMessage( "Welcome to eqPly\nPress F1 for help" );
    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit();
    _deregisterData();
    _closeAdminServer();

    // retain model & distributors for possible other config runs, dtor deletes
    return ret;
}

namespace
{
static bool _isPlyfile( const std::string& filename )
{
    const size_t size = filename.length();
    if( size < 5 )
        return false;

    if( filename[size-4] != '.' || filename[size-3] != 'p' ||
        filename[size-2] != 'l' || filename[size-1] != 'y' )
    {
        return false;
    }
    return true;
}
}

void Config::_loadModels()
{
    if( !_models.empty( )) // only load on the first config run
        return;

    eq::Strings filenames = _initData.getFilenames();
    while( !filenames.empty( ))
    {
        const std::string filename = filenames.back();
        filenames.pop_back();
     
        if( _isPlyfile( filename ))
        {
            Model* model = new Model;

            if( _initData.useInvertedFaces() )
                model->useInvertedFaces();
        
            if( !model->readFromFile( filename.c_str() ) )
            {
                EQWARN << "Can't load model: " << filename << std::endl;
                delete model;
            }
            else
                _models.push_back( model );
        }
        else
        {
            const std::string basename = co::base::getFilename( filename );
            if( basename == "." || basename == ".." )
                continue;

            // recursively search directories
            const eq::Strings subFiles = co::base::searchDirectory( filename,
                                                                    "*" );

            for(eq::StringsCIter i = subFiles.begin(); i != subFiles.end(); ++i)
                filenames.push_back( filename + '/' + *i );
        }
    }
}

void Config::_registerModels()
{
    // Register distribution helpers on each config run
    const bool createDist = _modelDist.empty(); //first run, create distributors
    const size_t  nModels = _models.size();
    EQASSERT( createDist || _modelDist.size() == nModels );

    for( size_t i = 0; i < nModels; ++i )
    {
        const Model* model = _models[i];
        ModelDist* modelDist = 0;
        if( createDist )
        {
            modelDist = new ModelDist( model );
            _modelDist.push_back( modelDist );
        }
        else
            modelDist = _modelDist[i];

        modelDist->registerTree( getClient( ));
        EQASSERT( modelDist->isAttached() );

        _frameData.setModelID( modelDist->getID( ));
    }

    EQASSERT( _modelDist.size() == nModels );

    if( !_modelDist.empty( ))
    {
        ModelAssigner assigner( _modelDist );
        accept( assigner );
    }
}

void Config::_deregisterData()
{
    for( ModelDistsCIter i = _modelDist.begin(); i != _modelDist.end(); ++i )
    {
        ModelDist* modelDist = *i;
        if( !modelDist->isAttached() ) // already done
            continue;

        EQASSERT( modelDist->isMaster( ));
        modelDist->deregisterTree();
    }

    deregisterObject( &_initData );
    deregisterObject( &_frameData );

    _initData.setFrameDataID( eq::UUID::ZERO );
    _frameData.setModelID( eq::UUID::ZERO );
}

bool Config::mapData( const eq::uint128_t& initDataID )
{
    if( !_initData.isAttached( ))
    {
        const uint32_t request = mapObjectNB( &_initData, initDataID,
                                              co::VERSION_OLDEST,
                                              getApplicationNode( ));
        if( !mapObjectSync( request ))
            return false;
        unmapObject( &_initData ); // data was retrieved, unmap immediately
    }
    else // appNode, _initData is registered already
    {
        EQASSERT( _initData.getID() == initDataID );
    }
    return true;
}

void Config::unmapData()
{
    for( ModelDistsCIter i = _modelDist.begin(); i != _modelDist.end(); ++i )
    {
        ModelDist* modelDist = *i;
        if( !modelDist->isAttached( )) // already done
            continue;

        if( !modelDist->isMaster( )) // leave registered on appNode
            modelDist->unmapTree();
    }
}

const Model* Config::getModel( const eq::uint128_t& modelID )
{
    if( modelID == eq::UUID::ZERO )
        return 0;

    // Protect if accessed concurrently from multiple pipe threads
    const eq::Node* node = getNodes().front();
    const bool needModelLock = (node->getPipes().size() > 1);
    co::base::ScopedMutex<> _mutex( needModelLock ? &_modelLock : 0 );

    const size_t nModels = _models.size();
    EQASSERT( _modelDist.size() == nModels );

    for( size_t i = 0; i < nModels; ++i )
    {
        const ModelDist* dist = _modelDist[ i ];
        if( dist->getID() == modelID )
            return _models[ i ];
    }
    
    _modelDist.push_back( new ModelDist );
    Model* model = _modelDist.back()->mapModel( getClient(), modelID );
    EQASSERT( model );
    _models.push_back( model );

    return model;
}

uint32_t Config::startFrame()
{
    _updateData();
    const eq::uint128_t& version = _frameData.commit();

    _redraw = false;
    return eq::Config::startFrame( version );
}

void Config::_updateData()
{
    // update head position
    if( _tracker.isRunning() )
    {
        _tracker.update();
        const eq::Matrix4f& headMatrix = _tracker.getMatrix();
        _setHeadMatrix( headMatrix );
    }

    // update database
    if( _animation.isValid( ))
    {
        const eq::Vector3f&  modelRotation = _animation.getModelRotation();
        const CameraAnimation::Step& curStep = _animation.getNextStep();

        _frameData.setModelRotation( modelRotation);
        _frameData.setRotation( curStep.rotation );
        _frameData.setCameraPosition( curStep.position );
    }
    else
    {
        if( _frameData.usePilotMode())
            _frameData.spinCamera( -0.001f * _spinX, -0.001f * _spinY );
        else
            _frameData.spinModel( -0.001f * _spinX, -0.001f * _spinY, 0.f );

        _frameData.moveCamera( 0.0f, 0.0f, 0.001f * _advance );
    }

    // idle mode
    if( isIdleAA( ))
    {
        EQASSERT( _numFramesAA > 0 );
        _frameData.setIdle( true );
    }
    else
        _frameData.setIdle( false );

    _numFramesAA = 0;
}

bool Config::isIdleAA()
{
    return ( !_needNewFrame() && _numFramesAA > 0 );
}

bool Config::needRedraw()
{
    return( _needNewFrame() || _numFramesAA > 0 );
}

uint32_t Config::getAnimationFrame()
{
    return _animation.getCurrentFrame();
}

bool Config::_needNewFrame()
{
    if( _messageTime > 0 )
    {
        if( getTime() - _messageTime > 2000 ) // reset message after two seconds
        {
            _messageTime = 0;
            _frameData.setMessage( "" );
        }
        return true;
    }

    return ( _spinX != 0 || _spinY != 0 || _advance != 0 ||
             _tracker.isRunning() || _redraw );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
            if( _handleKeyEvent( event->data.keyPress ))
            {
                _redraw = true;
                return true;
            }
            break;

        case eq::Event::CHANNEL_POINTER_BUTTON_PRESS:
        {
            const eq::uint128_t& viewID = event->data.context.view.identifier;
            _frameData.setCurrentViewID( viewID );
            if( viewID == eq::UUID::ZERO )
            {
                _currentCanvas = 0;
                return false;
            }
            
            const View* view = _getCurrentView();
            const eq::Layout* layout = view->getLayout();
            const eq::Canvases& canvases = getCanvases();
            for( eq::CanvasesCIter i = canvases.begin();
                 i != canvases.end(); ++i )
            {
                eq::Canvas* canvas = *i;
                const eq::Layout* canvasLayout = canvas->getActiveLayout();

                if( canvasLayout == layout )
                {
                    _currentCanvas = canvas;
                    return true;
                }
            }
            return true;
        }

        case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
        {
            const eq::PointerEvent& releaseEvent = 
                event->data.pointerButtonRelease;
            if( releaseEvent.buttons == eq::PTR_BUTTON_NONE)
            {
                if( releaseEvent.button == eq::PTR_BUTTON1 )
                {
                    _spinX = releaseEvent.dy;
                    _spinY = releaseEvent.dx;
                    _redraw = true;
                    return true;
                }
                if( releaseEvent.button == eq::PTR_BUTTON2 )
                {
                    _advance = -releaseEvent.dy;
                    _redraw = true;
                    return true;
                }
            }
            break;
        }
        case eq::Event::CHANNEL_POINTER_MOTION:
            switch( event->data.pointerMotion.buttons )
            {
              case eq::PTR_BUTTON1:
                  _spinX = 0;
                  _spinY = 0;

                  if( _frameData.usePilotMode())
                      _frameData.spinCamera(
                          -0.005f * event->data.pointerMotion.dy,
                          -0.005f * event->data.pointerMotion.dx );
                  else
                      _frameData.spinModel(
                          -0.005f * event->data.pointerMotion.dy,
                          -0.005f * event->data.pointerMotion.dx, 0.f );
                  _redraw = true;
                  return true;

              case eq::PTR_BUTTON2:
                  _advance = -event->data.pointerMotion.dy;
                  _frameData.moveCamera( 0.f, 0.f, .005f * _advance );
                  _redraw = true;
                  return true;

              case eq::PTR_BUTTON3:
                  _frameData.moveCamera(  .0005f * event->data.pointerMotion.dx,
                                         -.0005f * event->data.pointerMotion.dy,
                                          0.f );
                  _redraw = true;
                  return true;
            }
            break;

        case eq::Event::WINDOW_POINTER_WHEEL:
            _frameData.moveCamera( -0.05f * event->data.pointerWheel.yAxis,
                                   0.f,
                                   0.05f * event->data.pointerWheel.xAxis );
            _redraw = true;
            return true;

        case eq::Event::MAGELLAN_AXIS:
            _spinX = 0;
            _spinY = 0;
            _advance = 0;
            _frameData.spinModel(  0.0001f * event->data.magellan.zRotation,
                                  -0.0001f * event->data.magellan.xRotation,
                                  -0.0001f * event->data.magellan.yRotation );
            _frameData.moveCamera(  0.0001f * event->data.magellan.xAxis,
                                   -0.0001f * event->data.magellan.zAxis,
                                    0.0001f * event->data.magellan.yAxis );
            _redraw = true;
            return true;

        case eq::Event::MAGELLAN_BUTTON:
            if( event->data.magellan.button == eq::PTR_BUTTON1 )
                _frameData.toggleColorMode();

            _redraw = true;
            return true;

        case eq::Event::WINDOW_EXPOSE:
        case eq::Event::WINDOW_RESIZE:
        case eq::Event::WINDOW_CLOSE:
        case eq::Event::VIEW_RESIZE:
            _redraw = true;
            break;

        case ConfigEvent::IDLE_AA_LEFT:
            if( _useIdleAA )
            {
                const ConfigEvent* idleEvent = 
                    static_cast< const ConfigEvent* >( event );
                _numFramesAA = EQ_MAX( _numFramesAA, idleEvent->steps );
            }
            else
                _numFramesAA = 0;
            return false;

        case ConfigEvent::PIPE:
            _pipes.insert( event->data.originator );
            return false;

        default:
            break;
    }

    _redraw |= eq::Config::handleEvent( event );
    return _redraw;
}

bool Config::_handleKeyEvent( const eq::KeyEvent& event )
{
    switch( event.key )
    {
        case 't':
            _adjustTileSize( -1 );
            return true;
        case 'T':
            _adjustTileSize( 1 );
             return true;
        case 'u':
            _frameData.toggleCompression();
            return true;

        case 'n':
        case 'N':
            _frameData.togglePilotMode();
            return true;
        case ' ':
            stopFrames();
            _spinX   = 0;
            _spinY   = 0;
            _advance = 0;
            _frameData.reset();
            _setHeadMatrix( eq::Matrix4f::IDENTITY );
            return true;

        case 'i':
            _useIdleAA = !_useIdleAA;
            return true;

        case 'k':
        {
            co::base::RNG rng;
            if( rng.get< bool >( ))
                _frameData.toggleOrtho();
            if( rng.get< bool >( ))
                _frameData.toggleStatistics();
            if( rng.get< bool >( ))
                _switchCanvas();
            if( rng.get< bool >( ))
                _switchView();
            if( rng.get< bool >( ))
                _switchLayout( 1 );
            if( rng.get< bool >( ))
                _switchModel();
            if( rng.get< bool >( ))
                eqAdmin::addWindow( _getAdminServer(), rng.get< bool >( ));
            if( rng.get< bool >( ))
            {
                eqAdmin::removeWindow( _getAdminServer( ));
                _currentCanvas = 0;
            }
            if( rng.get< bool >( ))
                _switchViewMode();
            return true;
        }

        case 'o':
        case 'O':
            _frameData.toggleOrtho();
            return true;

        case 's':
        case 'S':
            _frameData.toggleStatistics();
            return true;
            
        case 'f':
            _freezeLoadBalancing( true );
            return true;

        case 'F':
            _freezeLoadBalancing( false );
            return true;

        case eq::KC_F1:
        case 'h':
        case 'H':
            _frameData.toggleHelp();
            return true;

        case 'd':
        case 'D':
            _frameData.toggleColorMode();
            return true;

        case 'q':
            _frameData.adjustQuality( -.1f );
            return true;

        case 'Q':
            _frameData.adjustQuality( .1f );
            return true;

        case 'c':
        case 'C':
            _switchCanvas();
            return true;

        case 'v':
        case 'V':
            _switchView();
            return true;

        case 'm':
        case 'M':
            _switchModel();
            return true;

        case 'l':
            _switchLayout( 1 );
            return true;
        case 'L':
            _switchLayout( -1 );
            return true;

        case 'w':
        case 'W':
            _frameData.toggleWireframe();
            return true;

        case 'r':
        case 'R':
            _frameData.toggleRenderMode();
            return true;
        case 'g':
        case 'G':
            _switchViewMode();
            return true;
        case 'a':
            eqAdmin::addWindow( _getAdminServer(), false /* active stereo */ );
            return true;
        case 'p':
            eqAdmin::addWindow( _getAdminServer(), true /* passive stereo */ );
            return true;
        case 'x':
            eqAdmin::removeWindow( _getAdminServer( ));
            _currentCanvas = 0;
            EQASSERT( update() );
            return false;

        // Head Tracking Emulation
        case eq::KC_UP:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.y() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_DOWN:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.y() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_RIGHT:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.x() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_LEFT:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.x() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_DOWN:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.z() += 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_UP:
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.z() -= 0.1f;
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '.':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_x( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ',':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_x( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ';':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_y( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '\'':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_y( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case '[':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_z( -.1f );
            _setHeadMatrix( headMatrix );
            return true;
        }
        case ']':
        {
            eq::Matrix4f headMatrix = _getHeadMatrix();
            headMatrix.pre_rotate_z( .1f );
            _setHeadMatrix( headMatrix );
            return true;
        }

        case '+':
            _changeFocusDistance( .1f );
            return true;

        case '-':
            _changeFocusDistance( -.1f );
            return true;

        case '1':
            _setFocusMode( eq::FOCUSMODE_FIXED );
            return true;

        case '2':
            _setFocusMode( eq::FOCUSMODE_RELATIVE_TO_ORIGIN );
            return true;

        case '3':
            _setFocusMode( eq::FOCUSMODE_RELATIVE_TO_OBSERVER );
            return true;

        case 'j':
            stopFrames();
            return true;

        case 'e':
            _toggleEqualizer();
            return true;

        default:
            return false;
    }
}

co::uint128_t Config::sync( const co::uint128_t& version )
{
    if( _admin.isValid() && _admin->isConnected( ))
        _admin->syncConfig( getID(), version );

    return eq::Config::sync( version );
}

void Config::_switchCanvas()
{
    const eq::Canvases& canvases = getCanvases();
    if( canvases.empty( ))
        return;

    _frameData.setCurrentViewID( eq::UUID::ZERO );

    if( !_currentCanvas )
    {
        _currentCanvas = canvases.front();
        return;
    }

    eq::CanvasesCIter i = stde::find( canvases, _currentCanvas );
    EQASSERT( i != canvases.end( ));

    ++i;
    if( i == canvases.end( ))
        _currentCanvas = canvases.front();
    else
        _currentCanvas = *i;
    _switchView(); // activate first view on canvas
}

void Config::_switchView()
{
    const eq::Canvases& canvases = getCanvases();
    if( !_currentCanvas && !canvases.empty( ))
        _currentCanvas = canvases.front();

    if( !_currentCanvas )
        return;

    const eq::Layout* layout = _currentCanvas->getActiveLayout();
    if( !layout )
        return;

    const View* view = _getCurrentView();
    const eq::Views& views = layout->getViews();
    EQASSERT( !views.empty( ));

    if( !view )
    {
        _frameData.setCurrentViewID( views.front()->getID( ));
        return;
    }

    eq::ViewsCIter i = std::find( views.begin(), views.end(), view );
    if( i != views.end( ))
        ++i;
    if( i == views.end( ))
        _frameData.setCurrentViewID( eq::UUID::ZERO );
    else
        _frameData.setCurrentViewID( (*i)->getID( ));
}

void Config::_switchModel()
{
    if( _modelDist.empty( )) // no models
        return;

    // current model of current view
    View* view = _getCurrentView();
    const eq::uint128_t& currentID = view ? view->getModelID() :
                                            _frameData.getModelID();
    // next model
    ModelDistsCIter i;
    for( i = _modelDist.begin(); i != _modelDist.end(); ++i )
    {
        if( (*i)->getID() != currentID )
            continue;
                
        ++i;
        break;
    }
    if( i == _modelDist.end( ))
        i = _modelDist.begin(); // wrap around

    // set identifier on view or frame data (default model)
    const eq::uint128_t& modelID = (*i)->getID();
    if( view )
        view->setModelID( modelID );
    else
        _frameData.setModelID( modelID );

    if( view )
    {
        const Model* model = getModel( modelID );
        _setMessage( "Using " + co::base::getFilename( model->getName( )));
    }
}

void Config::_switchViewMode()
{
    View* view = _getCurrentView();
    if( !view )
        return;

    const eq::View::Mode mode = view->getMode();
    if( mode == eq::View::MODE_MONO )
    {
        view->changeMode( eq::View::MODE_STEREO );
        _setMessage( "Switched to stereoscopic rendering" );
    }
    else
    {
        view->changeMode( eq::View::MODE_MONO );
        _setMessage( "Switched to monoscopic rendering" );
    }
}

void Config::_freezeLoadBalancing( const bool onOff )
{
    View* view = _getCurrentView();
    if ( view )
        view->freezeLoadBalancing( onOff );
}

void Config::_adjustTileSize( const int delta )
{
    View* view = _getCurrentView();
    if( !view )
        return;

    eq::Vector2i tileSize = view->getTileSize();
    if( tileSize == eq::Vector2i( -1, -1 ) )
        tileSize = eq::Vector2i( 64, 64 );
    tileSize += delta;
    view->setTileSize( tileSize );
}

void Config::_switchLayout( int32_t increment )
{
    if( !_currentCanvas )
        return;

    _frameData.setCurrentViewID( eq::UUID::ZERO );

    int64_t index = _currentCanvas->getActiveLayoutIndex() + increment;
    const eq::Layouts& layouts = _currentCanvas->getLayouts();
    EQASSERT( !layouts.empty( ));

    index = ( index % layouts.size( ));
    _currentCanvas->useLayout( uint32_t( index ));

    const eq::Layout* layout = layouts[index];
    std::ostringstream stream;
    stream << "Layout ";
    if( layout )
    {
        const std::string& name = layout->getName();
        if( name.empty( ))
            stream << index;
        else
            stream << name;
    }
    else
        stream << "NONE";

    stream << " active";
    _setMessage( stream.str( ));
}

void Config::_toggleEqualizer()
{
    View* view = _getCurrentView();
    if ( view )
        view->toggleEqualizer();
}

// Note: real applications would use one tracking device per observer
void Config::_setHeadMatrix( const eq::Matrix4f& matrix )
{
    const eq::Observers& observers = getObservers();
    for( eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i )
    {
        (*i)->setHeadMatrix( matrix );
    }

    eq::Vector3f trans;
    matrix.get_translation( trans );
    std::ostringstream stream;
    stream << "Observer at " << trans;
    _setMessage( stream.str( ));
}

const eq::Matrix4f& Config::_getHeadMatrix() const
{
    const eq::Observers& observers = getObservers();
    if( observers.empty( ))
        return eq::Matrix4f::IDENTITY;

    return observers[0]->getHeadMatrix();
}

void Config::_changeFocusDistance( const float delta )
{
    const eq::Observers& observers = getObservers();
    for( eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i )
    {
        eq::Observer* observer = *i;
        observer->setFocusDistance( observer->getFocusDistance() + delta );
        std::ostringstream stream;
        stream << "Set focus distance to " << observer->getFocusDistance();
        _setMessage( stream.str( ));
    }
}

void Config::_setFocusMode( const eq::FocusMode mode )
{
    const eq::Observers& observers = getObservers();
    for( eq::ObserversCIter i = observers.begin(); i != observers.end(); ++i )
        (*i)->setFocusMode( mode );

    std::ostringstream stream;
    stream << "Set focus mode to " << mode;
    _setMessage( stream.str( ));
}

void Config::_setMessage( const std::string& message )
{
    _frameData.setMessage( message );
    _messageTime = getTime();
}

eq::admin::ServerPtr Config::_getAdminServer()
{
    // Debug: _closeAdminServer();
    if( _admin.isValid() && _admin->isConnected( ))
        return _admin;

    eq::admin::init( 0, 0 );
    eq::admin::ClientPtr client = new eq::admin::Client;
    if( !client->initLocal( 0, 0 ))
    {
        _setMessage( "Can't init admin client" );
        eq::admin::exit();
    }

    _admin = new eq::admin::Server;
    if( !client->connectServer( _admin ))
    {
        _setMessage( "Can't open connection to administrate server" );
        client->exitLocal();
        _admin = 0;
        eq::admin::exit();
    }
    return _admin;
}

void Config::_closeAdminServer()
{
    if( !_admin )
        return;

    eq::admin::ClientPtr client = _admin->getClient();
    client->disconnectServer( _admin );
    client->exitLocal();
    EQASSERT( client->getRefCount() == 1 );
    EQASSERT( _admin->getRefCount() == 1 );
    
    _admin = 0;
    eq::admin::exit();
}

View* Config::_getCurrentView()
{
    const eq::uint128_t& viewID = _frameData.getCurrentViewID();
    eq::View* view = find< eq::View >( viewID );
    return static_cast< View* >( view );
}

const View* Config::_getCurrentView() const
{
    const eq::uint128_t& viewID = _frameData.getCurrentViewID();
    const eq::View* view = find< eq::View >( viewID );
    return static_cast< const View* >( view );
}

}
