
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "view.h"

#include "modelAssigner.h"
#include "layoutSwitcher.h"
#include "nextViewFinder.h"

using namespace std;

namespace eqPly
{

Config::Config( eq::base::RefPtr< eq::Server > parent )
        : eq::Config( parent )
        , _spinX( 5 )
        , _spinY( 5 )
        , _redraw( true )
{
}

Config::~Config()
{
    for( ModelVector::const_iterator i = _models.begin(); 
         i != _models.end(); ++i )
    {
        delete *i;
    }
    _models.clear();

    for( ModelDistVector::const_iterator i = _modelDist.begin(); 
         i != _modelDist.end(); ++i )
    {
        EQASSERT( (*i)->getID() == EQ_ID_INVALID );
        delete *i;
    }
    _modelDist.clear();
}

bool Config::init()
{
    _loadModels();

    // init distributed objects
    _frameData.setColor( _initData.useColor( ));
    _frameData.setRenderMode( _initData.getRenderMode( ));
    registerObject( &_frameData );
    
    _initData.setFrameDataID( _frameData.getID( ));
    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
    {
        _deregisterData();
        return false;
    }

    // init tracker
    if( !_initData.getTrackerPort().empty( ))
    {
        if( !_tracker.init( _initData.getTrackerPort() ))
            EQWARN << "Failed to initialise tracker" << endl;
        else
        {
            // Set up position of tracking system wrt world space
            // Note: this depends on the physical installation.
            vmml::Matrix4f m( vmml::Matrix4f::IDENTITY );
            m.scale( 1.f, 1.f, -1.f );
            _tracker.setWorldToEmitter( m );

            m = vmml::Matrix4f::IDENTITY;
            m.rotateZ( -M_PI_2 );
            _tracker.setSensorToObject( m );
            EQINFO << "Tracker initialized" << endl;
        }
    }

    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit();
    _deregisterData();

    // retain models and distributors for possible other config runs, destructor
    // deletes it
    return ret;
}

void Config::_loadModels()
{
    if( _models.empty( )) // only load on the first config run
    {
        const std::vector< std::string >& filenames = _initData.getFilenames();
        for(  std::vector< std::string >::const_iterator i = filenames.begin();
              i != filenames.end(); ++i )
        {
            const std::string& filename = *i;
            EQINFO << "Loading model " << filename << endl;
     
            Model* model = new Model;

            if( _initData.useInvertedFaces() )
                model->useInvertedFaces();
        
            if ( !model->readFromFile( filename.c_str() ) )
            {
                EQWARN << "Can't load model: " << filename << endl;
                delete model;
            }
            else
                _models.push_back( model );
        }
    }
    
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

        modelDist->registerTree( this );
        EQASSERT( modelDist->getID() != EQ_ID_INVALID );

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
    for( ModelDistVector::const_iterator i = _modelDist.begin(); 
         i != _modelDist.end(); ++i )
    {
        ModelDist* modelDist = *i;
        if( modelDist->getID() == EQ_ID_INVALID ) // already done
            continue;

        EQASSERT( modelDist->isMaster( ));
        modelDist->deregisterTree();
    }

    deregisterObject( &_initData );
    deregisterObject( &_frameData );

    _initData.setFrameDataID( EQ_ID_INVALID );
    _frameData.setModelID( EQ_ID_INVALID );
}


void Config::mapData( const uint32_t initDataID )
{
    if( _initData.getID() == EQ_ID_INVALID )
    {
        EQCHECK( mapObject( &_initData, initDataID ));
        unmapObject( &_initData ); // data was retrieved, unmap immediately
    }
    else  // appNode, _initData is registered already
        EQASSERT( _initData.getID() == initDataID );
}

void Config::unmapData()
{
    for( ModelDistVector::const_iterator i = _modelDist.begin(); 
         i != _modelDist.end(); ++i )
    {
        ModelDist* modelDist = *i;
        if( modelDist->getID() == EQ_ID_INVALID ) // already done
            continue;

        if( !modelDist->isMaster( )) // leave registered on appNode
            modelDist->unmapTree();
    }
}

const Model* Config::getModel( const uint32_t modelID )
{
    if( modelID == EQ_ID_INVALID ) // no model loaded by application
        return 0;

    // Accessed concurrently from pipe threads
    eq::base::ScopedMutex< eq::base::SpinLock > _mutex( _modelLock );

    const size_t nModels = _models.size();
    EQASSERT( _modelDist.size() == nModels );

    for( size_t i = 0; i < nModels; ++i )
    {
        const ModelDist* dist = _modelDist[ i ];
        if( dist->getID() == modelID )
            return _models[ i ];
    }
    
    _modelDist.push_back( new ModelDist );
    Model* model = _modelDist.back()->mapModel( this, modelID );
    EQASSERT( model );
    _models.push_back( model );

    return model;
}


uint32_t Config::startFrame()
{
    // update head position
    if( _tracker.isRunning() )
    {
        _tracker.update();
        const vmml::Matrix4f& headMatrix = _tracker.getMatrix();
        setHeadMatrix( headMatrix );
    }

    // update database
    _frameData.spinCamera( -0.001f * _spinX, -0.001f * _spinY );
    const uint32_t version = _frameData.commit();

    _redraw = false;
    return eq::Config::startFrame( version );
}

bool Config::needsRedraw()
{
    return ( _spinX != 0 || _spinY != 0 || _tracker.isRunning() || _redraw );
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

        case eq::Event::POINTER_BUTTON_PRESS:
            _frameData.setCurrentViewID( event->data.context.view.id );
            return true;

        case eq::Event::POINTER_BUTTON_RELEASE:
            if( event->data.pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE
                && event->data.pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinX = event->data.pointerButtonRelease.dx;
                _spinY = event->data.pointerButtonRelease.dy;
                _redraw = true;
            }
            return true;

        case eq::Event::POINTER_MOTION:
            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                _frameData.spinCamera( -0.005f * event->data.pointerMotion.dx,
                                       -0.005f * event->data.pointerMotion.dy );
                _redraw = true;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->data.pointerMotion.buttons == ( eq::PTR_BUTTON1 |
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData.moveCamera( 0.f, 0.f,
                                       .005f * event->data.pointerMotion.dy );
                _redraw = true;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData.moveCamera(  .0005f * event->data.pointerMotion.dx,
                                       -.0005f * event->data.pointerMotion.dy,
                                       0.f );
                _redraw = true;
            }
            return true;

        case eq::Event::EXPOSE:
        case eq::Event::WINDOW_RESIZE:
        case eq::Event::WINDOW_CLOSE:
        case eq::Event::VIEW_RESIZE:
            _redraw = true;
            break;

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
        case ' ':
            _spinX = 0;
            _spinY = 0;
            _frameData.reset();
            setHeadMatrix( vmml::Matrix4f::IDENTITY );
            return true;

        case 'o':
        case 'O':
            _frameData.toggleOrtho();
            return true;

        case 's':
        case 'S':
            _frameData.toggleStatistics();
            return true;
            
        case eq::KC_F1:
        case 'h':
        case 'H':
            _frameData.toggleHelp();
            return true;
            
        case 'v':
        case 'V':
        {
            NextViewFinder finder( _frameData.getCurrentViewID( ));
            accept( finder );
            const eq::View* view = finder.getResult();
            if( view )
                _frameData.setCurrentViewID( view->getID( ));
            else
                _frameData.setCurrentViewID( EQ_ID_INVALID );
            return true;
        }

        case 'm':
        case 'M':
        {
            if( _modelDist.empty( )) // no models
                return true;

            const uint32_t viewID = _frameData.getCurrentViewID();
            View* view = static_cast< View* >( findView( viewID ));
            const uint32_t currentID = view ? 
                view->getModelID() : _frameData.getModelID();
            ModelDistVector::const_iterator i;
            for( i = _modelDist.begin(); i != _modelDist.end(); ++i )
            {
                if( (*i)->getID() != currentID )
                    continue;
                
                ++i;
                break;
            }
            if( i == _modelDist.end( ))
                i = _modelDist.begin(); // wrap around
            
            const uint32_t modelID = (*i)->getID();
            if( view )
                view->setModelID( modelID );
            else
                _frameData.setModelID( modelID );
            return true;
        }

        case 'l':
        case 'L':
        {
            const uint32_t viewID = _frameData.getCurrentViewID();
            LayoutSwitcher switcher( viewID );
            accept( switcher );
            
            _frameData.setCurrentViewID( EQ_ID_INVALID );
            const eq::Layout* layout = switcher.getResult();
            if( layout )
            {
                const eq::ViewVector& views = layout->getViews();
                if( !views.empty( ))
                    _frameData.setCurrentViewID( views[0]->getID( ));
            }
            return true;
        }

        case 'w':
        case 'W':
            _frameData.toggleWireframe();
            return true;

        case 'r':
        case 'R':
            _frameData.toggleRenderMode();
            return true;

        // Head Tracking Emulation
        case eq::KC_UP:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.y += 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_DOWN:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.y -= 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_RIGHT:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.x += 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_LEFT:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.x -= 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_DOWN:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.z += 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }
        case eq::KC_PAGE_UP:
        {
            vmml::Matrix4f headMatrix = getHeadMatrix();
            headMatrix.z -= 0.1f;
            setHeadMatrix( headMatrix );
            return true;
        }

        default:
            return false;
    }
}
}
