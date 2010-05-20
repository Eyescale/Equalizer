
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "config.h"

using namespace std;

namespace eVolve
{

Config::Config( eq::base::RefPtr< eq::Server > parent )
        : eq::Config( parent )
        , _spinX( 5 )
        , _spinY( 5 )
        , _currentCanvas( 0 )
        , _messageTime( 0 )
{
}

Config::~Config()
{
}

bool Config::init()
{
    // init distributed objects
    registerObject( &_frameData );

    _frameData.setOrtho( _initData.getOrtho( ));
    _initData.setFrameDataID( _frameData.getID( ));

    _frameData.setAutoObsolete( getLatency( ));

    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
    {
        _deregisterData();
        return false;
    }

    const eq::CanvasVector& canvases = getCanvases();
    if( canvases.empty( ))
        _currentCanvas = 0;
    else
        _currentCanvas = canvases.front();

    _setMessage( "Welcome to eVolve\nPress F1 for help" );

    return true;
}

void Config::mapData( const uint32_t initDataID )
{
    if( _initData.getID() == EQ_ID_INVALID )
    {
        EQCHECK( mapObject( &_initData, initDataID ));
        unmapObject( &_initData ); // data was retrieved, unmap immediately
    }
    else  // appNode, _initData is registered already
    {
        EQASSERT( _initData.getID() == initDataID );
    }
}


bool Config::exit()
{
    const bool ret = eq::Config::exit();
    _deregisterData();

    return ret;
}

void Config::_deregisterData()
{
    deregisterObject( &_initData );
    deregisterObject( &_frameData );

    _initData.setFrameDataID( EQ_ID_INVALID );
}


uint32_t Config::startFrame()
{
    // update database
    _frameData.spinCamera( -0.001f * _spinX, -0.001f * _spinY );
    const uint32_t version = _frameData.commit();

    _resetMessage();

    return eq::Config::startFrame( version );
}

void Config::_resetMessage()
{
    // reset message after two seconds
    if( _messageTime > 0 && getTime() - _messageTime > 2000 )
    {
        _messageTime = 0;
        _frameData.setMessage( "" );
    }
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
            if( _handleKeyEvent( event->data.keyPress ))
                return true;
            break;

        case eq::Event::POINTER_BUTTON_PRESS:
        {
            const uint32_t viewID = event->data.context.view.identifier;
            _frameData.setCurrentViewID( viewID );
            if( viewID == EQ_ID_INVALID )
            {
                _currentCanvas = 0;
                return true;
            }
            
            const eq::View* view = find< eq::View >( viewID );
            const eq::Layout* layout = view->getLayout();
            const eq::CanvasVector& canvases = getCanvases();
            for( eq::CanvasVector::const_iterator i = canvases.begin();
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

        case eq::Event::POINTER_BUTTON_RELEASE:
            if( event->data.pointerButtonRelease.buttons == eq::PTR_BUTTON_NONE
                && event->data.pointerButtonRelease.button  == eq::PTR_BUTTON1 )
            {
                _spinY = event->data.pointerButtonRelease.dx;
                _spinX = event->data.pointerButtonRelease.dy;
            }
            return true;

        case eq::Event::POINTER_MOTION:
            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON_NONE )
                return true;

            if( event->data.pointerMotion.buttons == eq::PTR_BUTTON1 )
            {
                _spinX = 0;
                _spinY = 0;

                _frameData.spinCamera(  -0.005f * event->data.pointerMotion.dy,
                                        -0.005f * event->data.pointerMotion.dx);
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON2 ||
                     event->data.pointerMotion.buttons == ( eq::PTR_BUTTON1 |
                                                       eq::PTR_BUTTON3 ))
            {
                _frameData.moveCamera( .0, .0,
                                        .005f*event->data.pointerMotion.dy);
            }
            else if( event->data.pointerMotion.buttons == eq::PTR_BUTTON3 )
            {
                _frameData.moveCamera( .0005f * event->data.pointerMotion.dx,
                                      -.0005f * event->data.pointerMotion.dy, 
                                       .0 );
            }
            return true;

        default:
            break;
    }
    return eq::Config::handleEvent( event );
}

bool Config::_handleKeyEvent( const eq::KeyEvent& event )
{
    switch( event.key )
    {
        case eq::KC_F1:
        case 'h':
        case 'H':
            _frameData.toggleHelp();
            return true;

        case 'r':
        case 'R':
        case ' ':
            _spinX = 0;
            _spinY = 0;
            _frameData.reset();
            return true;

        case 'o':
        case 'O':
            _frameData.toggleOrtho();
            return true;

        case 's':
        case 'S':
        _frameData.toggleStatistics();
            return true;

        case 'l':
            _switchLayout( 1 );
            return true;
        case 'L':
            _switchLayout( -1 );
            return true;

        case 'q':
            _frameData.adjustQuality( -.1f );
            return true;

        case 'Q':
            _frameData.adjustQuality( .1f );
            return true;

        case 'c':
        case 'C':
        {
            const eq::CanvasVector& canvases = getCanvases();
            if( canvases.empty( ))
                return true;

            _frameData.setCurrentViewID( EQ_ID_INVALID );

            if( !_currentCanvas )
            {
                _currentCanvas = canvases.front();
                return true;
            }

            eq::CanvasVector::const_iterator i = 
                    std::find( canvases.begin(), canvases.end(),
                                                   _currentCanvas );
            EQASSERT( i != canvases.end( ));

            ++i;
            if( i == canvases.end( ))
                _currentCanvas = canvases.front();
            else
                _currentCanvas = *i;
            return true;
        }

        case 'v':
        case 'V':
        {
            const eq::CanvasVector& canvases = getCanvases();
            if( !_currentCanvas && !canvases.empty( ))
                _currentCanvas = canvases.front();

            if( !_currentCanvas )
                return true;

            const eq::Layout* layout = _currentCanvas->getActiveLayout();
            if( !layout )
                return true;

            const eq::View* current = 
                              find< eq::View >( _frameData.getCurrentViewID( ));

            const eq::ViewVector& views = layout->getViews();
            EQASSERT( !views.empty( ))

            if( !current )
            {
                _frameData.setCurrentViewID( views.front()->getID( ));
                return true;
            }

            eq::ViewVector::const_iterator i = std::find( views.begin(),
                                                          views.end(),
                                                          current );
            EQASSERT( i != views.end( ));

            ++i;
            if( i == views.end( ))
                _frameData.setCurrentViewID( EQ_ID_INVALID );
            else
                _frameData.setCurrentViewID( (*i)->getID( ));
            return true;
        }

        default:
            break;
    }
    return false;
}

void Config::_setMessage( const std::string& message )
{
    _frameData.setMessage( message );
    _messageTime = getTime();
}

void Config::_switchLayout( int32_t increment )
{
    if( !_currentCanvas )
        return;

    _frameData.setCurrentViewID( EQ_ID_INVALID );

    int32_t index = _currentCanvas->getActiveLayoutIndex() + increment;
    const eq::LayoutVector& layouts = _currentCanvas->getLayouts();
    EQASSERT( !layouts.empty( ))

        if( index >= static_cast<int32_t>(layouts.size( )) )
            index = 0;
        else if ( index < 0 )
            index = layouts.size( ) - 1;

    _currentCanvas->useLayout( index );

    const eq::Layout* layout = _currentCanvas->getLayouts()[index];
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

}
