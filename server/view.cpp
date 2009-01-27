
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include "config.h"
#include "viewData.h"

using namespace eq::base;

namespace eq
{
namespace server
{

View::View()
        : _data( *(new ViewData) )
{
}

View::View( const View& from )
        : eq::View( from )
        , _data( from._data )
        , _vp( from._vp )
{
}

void View::setViewport( const eq::Viewport& vp )
{
    _vp = vp;
}

//----- old View API below
View::View( ViewData& data )
        : _data( data )
{
    _updateView();
}

View::View( const View& from, ViewData& data )
        : eq::View( from )
        , _data( data )
        , _vp( from._vp )
{
    _updateView();
}

void View::setWall( const eq::Wall& wall )
{
    eq::View::setWall( wall );
    _updateView();
}
        
void View::setProjection( const eq::Projection& projection )
{
    eq::View::setProjection( projection );
    _updateView();
}

void View::setEyeBase( const float eyeBase )
{
    eq::View::setEyeBase( eyeBase );
    updateHead();
}

void View::applyInstanceData( net::DataIStream& is )
{
    eq::View::applyInstanceData( is );

    if( _dirty == DIRTY_NONE )
        _data.invalidate();

    if( _dirty & (DIRTY_WALL | DIRTY_PROJECTION) )
        _updateView();
    if( _dirty & DIRTY_EYEBASE )
        updateHead();
}

void View::_updateView()
{
    switch( getCurrentType( ))
    {
        case TYPE_WALL:
            _data.applyWall( getWall( ));
            break;
        case TYPE_PROJECTION:
            _data.applyProjection( getProjection( ));
            break;

        case TYPE_NONE:
            _data.invalidate();
            break;
        default:
            EQUNREACHABLE;
    }
}

void View::updateHead()
{
    const net::Session* session = getSession();
    if( !session ) // not yet mapped (active)
        return;

    // XXX eye base and head matrix belong together, see Issues on view spec
    const Config* config = EQSAFECAST( const Config*, session );
    _data.applyHead( config->getHeadMatrix(), getEyeBase( ));
}

std::ostream& operator << ( std::ostream& os, const View* view )
{
    if( !view )
        return os;
    
    os << disableFlush << disableHeader << "view" << std::endl;
    os << "{" << std::endl << indent;
    
    const std::string& name = view->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const eq::Viewport& vp  = view->getViewport();
    if( vp.isValid( ) && vp != eq::Viewport::FULL )
        os << "viewport " << vp << std::endl;

    switch( view->getCurrentType( ))
    {
        case eq::View::TYPE_WALL:
            os << view->getWall() << std::endl;
            break;
        case eq::View::TYPE_PROJECTION:
            os << view->getProjection() << std::endl;
            break;
        default: 
            break;
    }

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
