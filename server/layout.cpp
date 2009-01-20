
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "layout.h"

#include "layoutVisitor.h"
#include "view.h"

namespace eq
{
namespace server
{

Layout::Layout( const Layout& from )
        : _name( from._name )
{
    for( ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        _views.push_back( new View( **i ));
    }
}

Layout::~Layout()
{
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
        delete *i;
    _views.clear();
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

}
}
