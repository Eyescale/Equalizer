
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
        , _modelDist( 0 )
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

    delete _modelDist;
    _modelDist = 0;
}

bool Config::init()
{
    _loadModel();

    // init distributed objects
    _frameData.setColor( _initData.useColor( ));
    _frameData.setRenderMode( _initData.getRenderMode( ));
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
    if( _models.empty( ))
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
    
    // TODO: move to View
    if( !_models.empty() && !_modelDist )
    {
        _modelDist = new ModelDist( _models[0] );
        _modelDist->registerTree( this );
        EQASSERT( _modelDist->getID() != EQ_ID_INVALID );

        _initData.setModelID( _modelDist->getID( ));
    }
}

bool Config::mapData( const uint32_t initDataID )
{
    if( _initData.getID() == EQ_ID_INVALID )
    {
        EQCHECK( mapObject( &_initData, initDataID ));
        unmapObject( &_initData ); // data was retrieved, unmap immediately
    }
    else  // appNode, _initData is registered already
        EQASSERT( _initData.getID() == initDataID );

    const uint32_t modelID = _initData.getModelID();
    if( modelID == EQ_ID_INVALID ) // no model loaded by application
        return true;

    if( _models.empty( ))
    {
        Model* model = ModelDist::mapModel( this, modelID );
        if( model )
            _models.push_back( model );
    }

    return !_models.empty();
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

namespace
{
class NextViewFinder : public eq::ConfigVisitor
{
public:
    NextViewFinder( const uint32_t currentViewID ) 
            : _id( currentViewID ), _layout( 0 ), _result( 0 )
            , _stopNext( false ) {}
    virtual ~NextViewFinder(){}

    virtual eq::VisitorResult visitPre( eq::Canvas* canvas )
        {
            _layout = canvas->getLayout();
            if( _layout )
                _layout->accept( this );

            _layout = 0;
            return eq::TRAVERSE_PRUNE;
        }

    virtual eq::VisitorResult visitPre( eq::Layout* layout )
        {
            if( _layout != layout )
                return eq::TRAVERSE_PRUNE; // only consider used layouts
            return eq::TRAVERSE_CONTINUE; 
        }

    virtual eq::VisitorResult visit( eq::View* view )
        {
            if( _stopNext || _id == EQ_ID_INVALID )
            {
                _result = view;
                return eq::TRAVERSE_TERMINATE;
            }
            
            if( view->getID() == _id )
                _stopNext = true;    
            return eq::TRAVERSE_CONTINUE; 
        }

    const eq::View* getResult() const { return _result; }

private:
    const uint32_t _id;
    eq::Layout*    _layout;
    eq::View*      _result;
    bool           _stopNext;
};
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
            _redraw = true;
            switch( event->data.keyPress.key )
            {
                case 'r':
                case 'R':
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

                case 'v':
                case 'V':
                {
                    NextViewFinder finder( _frameData.getCurrentViewID( ));
                    accept( &finder );
                    const eq::View* view = finder.getResult();
                    if( view )
                        _frameData.setCurrentViewID( view->getID( ));
                    else
                        _frameData.setCurrentViewID( EQ_ID_INVALID );
                    return true;
                }

                case 'w':
                case 'W':
                    _frameData.toggleWireframe();
                    return true;

                case 'm':
                case 'M':
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
                    break;
            }
            break;

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
                _frameData.moveCamera( .0005f * event->data.pointerMotion.dx,
                                       .0005f * event->data.pointerMotion.dy,
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

    return eq::Config::handleEvent( event );
}
}
