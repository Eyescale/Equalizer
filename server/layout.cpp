
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "layout.h"

#include "config.h"
#include "layoutVisitor.h"
#include "paths.h"
#include "view.h"

using namespace eq::base;

namespace eq
{
namespace server
{

Layout::Layout()
        : _config( 0 )
{}

Layout::Layout( const Layout& from, Config* config )
        : _config( 0 )
{
    config->addLayout( this );
    EQASSERT( _config );

    for( ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        addView( new View( **i ));
    }
}

Layout::~Layout()
{
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        removeView( view );
        delete view;
    }

    _views.clear();
}

LayoutPath Layout::getPath() const
{
    EQASSERT( _config );
    
    const LayoutVector&    layouts = _config->getLayouts();
    LayoutVector::const_iterator i = std::find( layouts.begin(), layouts.end(),
                                              this );
    EQASSERT( i != layouts.end( ));

    LayoutPath path;
    path.layoutIndex = std::distance( layouts.begin(), i );
    return path;
}

VisitorResult Layout::accept( LayoutVisitor* visitor )
{ 
    VisitorResult result = visitor->visitPre( this );
    if( result != TRAVERSE_CONTINUE )
        return result;

    for( ViewVector::const_iterator i = _views.begin(); 
         i != _views.end(); ++i )
    {
        View* view = *i;
        switch( view->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor->visitPost( this ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}

void Layout::addView( View* view )
{
    EQASSERT( view );
    if( view->getName().empty( ))
    {
        std::stringstream name;
        name << "view" << _views.size() + 1;
        view->setName( name.str( ));
    }

    view->_layout = this;
    _views.push_back( view );
}

bool Layout::removeView( View* view )
{
    ViewVector::iterator i = find( _views.begin(), _views.end(), view );
    if( i == _views.end( ))
        return false;

    _views.erase( i );
    view->_layout = 0;
    return true;
}


std::ostream& operator << ( std::ostream& os, const Layout* layout )
{
    if( !layout )
        return os;
    
    os << disableFlush << disableHeader << "layout" << std::endl;
    os << "{" << std::endl << indent; 

    const std::string& name = layout->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const ViewVector& views = layout->getViews();
    for( ViewVector::const_iterator i = views.begin(); i != views.end(); ++i )
        os << *i;

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
