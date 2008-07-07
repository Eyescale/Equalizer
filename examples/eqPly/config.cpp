
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

using namespace std;

namespace eqPly
{

Config::Config( eq::base::RefPtr< eq::Server > parent )
        : eq::Config( parent )
        , _spinX( 5 )
        , _spinY( 5 )
        , _model( 0 )
        , _modelDist( 0 )
{
}

Config::~Config()
{
    delete _model;
    _model = 0;

    delete _modelDist;
    _modelDist = 0;
}

bool Config::init()
{
    _loadModel();

    // init distributed objects
    _frameData.data.color      = _initData.useColor();
    _frameData.data.renderMode = _initData.getRenderMode();
    registerObject( &_frameData );
    
    _initData.setFrameDataID( _frameData.getID( ));
    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
        return false;
    
    // init tracker
    if( !_initData.getTrackerPort().empty( ))
    {
        if( !_tracker.init( _initData.getTrackerPort() ))
            EQWARN << "Failed to initialise tracker" << endl;
        else
        {
            // Set up position of tracking system in world space
            // Note: this depends on the physical installation.
            vmml::Matrix4f m( vmml::Matrix4f::IDENTITY );
            m.scale( 1.f, 1.f, -1.f );
            //m.x = .5;
            _tracker.setWorldToEmitter( m );

            m = vmml::Matrix4f::IDENTITY;
            m.rotateZ( -M_PI_2 );
            _tracker.setSensorToObject( m );
            EQINFO << "Tracker initialised" << endl;
        }
    }

    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit();

    _initData.setFrameDataID( EQ_ID_INVALID );
    deregisterObject( &_initData );
    deregisterObject( &_frameData );

    if( _modelDist )
    {
        _modelDist->deregisterTree();
        delete _modelDist;
        _modelDist = 0;

        _initData.setModelID( EQ_ID_INVALID );
        // retain model for possible other config runs, destructor deletes it
    }

    return ret;
}

void Config::_loadModel()
{
    if( !_model )
    {
        const string& filename = _initData.getFilename();
        EQINFO << "Loading model " << filename << endl;
     
        _model = new Model;

        if( _initData.useInvertedFaces() )
            _model->useInvertedFaces();
        
        if ( !_model->readFromFile( filename.c_str() ) )
        {
            EQWARN << "Can't load model: " << filename << endl;

            delete _model;
            _model = 0;
            return;
        }
    }
    
    if( !_modelDist )
    {
        EQASSERT( _model );
        _modelDist = new ModelDist( _model );
        _modelDist->registerTree( this );
        EQASSERT( _modelDist->getID() != EQ_ID_INVALID );

        _initData.setModelID( _modelDist->getID( ));
    }
}

bool Config::mapData( const uint32_t initDataID )
{
    if( _initData.getID() == EQ_ID_INVALID )
    {
        const bool mapped = mapObject( &_initData, initDataID );
        EQASSERT( mapped );
        unmapObject( &_initData ); // data was retrieved, unmap immediately
    }
    else  // appNode, _initData is registered already
        EQASSERT( _initData.getID() == initDataID );

    const uint32_t modelID = _initData.getModelID();
    if( modelID == EQ_ID_INVALID ) // no model loaded by application
        return true;

    if( !_model )
        _model = ModelDist::mapModel( this, modelID );

    return (_model != 0);
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
    _frameData.data.rotation.preRotateX( -0.001f * _spinX );
    _frameData.data.rotation.preRotateY( -0.001f * _spinY );
    const uint32_t version = _frameData.commit();

    return eq::Config::startFrame( version );
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
            switch( event->data.keyPress.key )
            {
                case 'r':
                case 'R':
                case ' ':
                    _spinX = 0;
                    _spinY = 0;
                    _frameData.reset();
                    return true;

                case 'o':
                case 'O':
                    _frameData.data.ortho = !_frameData.data.ortho;
                    return true;

                case 's':
                case 'S':
                    _frameData.data.statistics = !_frameData.data.statistics;
                    return true;

                case 'm':
                case 'M':
                    _frameData.data.renderMode = static_cast<mesh::RenderMode>
                        ((_frameData.data.renderMode+1)%mesh::RENDER_MODE_ALL);
                    EQINFO << "Switched to " << _frameData.data.renderMode
                           << endl;
                    return true;

                default:
                    break;
            }
            break;

        case eq::Event::POINTER_BUTTON_RELEASE:
            if( event->data.pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE
                && event->data.pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinX = event->data.pointerButtonRelease.dx;
                _spinY = event->data.pointerButtonRelease.dy;
            }
            return true;

        case eq::Event::POINTER_MOTION:
            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                _frameData.data.rotation.preRotateX( 
                    -0.005f * event->data.pointerMotion.dx );
                _frameData.data.rotation.preRotateY(
                    -0.005f * event->data.pointerMotion.dy );
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->data.pointerMotion.buttons == ( eq::PTR_BUTTON1 |
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData.data.translation.z +=
                    .005f * event->data.pointerMotion.dy;
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData.data.translation.x += 
                    .0005f * event->data.pointerMotion.dx;
                _frameData.data.translation.y -= 
                    .0005f * event->data.pointerMotion.dy;
            }
            return true;

        default:
            break;
    }
    return eq::Config::handleEvent( event );
}
}
