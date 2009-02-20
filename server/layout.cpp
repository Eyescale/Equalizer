
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "layout.h"

#include "config.h"
#include "layoutVisitor.h"
#include "nameFinder.h"
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
    for( ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        addView( new View( **i, config ));
    }

    config->addLayout( this );
    EQASSERT( _config );
}

Layout::~Layout()
{
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        view->_layout = 0;
        delete view;
    }
    _views.clear();
}

void Layout::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_ALL ) // children are immutable
    {
        for( ViewVector::const_iterator i = _views.begin();
             i != _views.end(); ++i )
        {
            View* view = *i;
            EQASSERT( view->getID() != EQ_ID_INVALID );
            os << view->getID();
        }
        os << EQ_ID_INVALID;
    }
}

View* Layout::getView( const ViewPath& path )
{
    EQASSERTINFO( _views.size() >= path.viewIndex,
                  _views.size() << " < " << path.viewIndex );

    if( _views.size() < path.viewIndex )
        return 0;

    return _views[ path.viewIndex ];
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

namespace
{
template< class C, class V >
VisitorResult _accept( C* layout, V& visitor )
{ 
    VisitorResult result = visitor.visitPre( layout );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const ViewVector& views = layout->getViews();
    for( ViewVector::const_iterator i = views.begin(); i != views.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
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

    switch( visitor.visitPost( layout ))
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

VisitorResult Layout::accept( LayoutVisitor& visitor )
{
    return _accept( this, visitor );
}
VisitorResult Layout::accept( ConstLayoutVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Layout::addView( View* view )
{
    EQASSERT( view );
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

View* Layout::findView( const std::string& name )
{
    ViewFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Layout::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        EQASSERT( view->getID() != EQ_ID_INVALID );
        
        session->unmapObject( view );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    session->unmapObject( this );
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
