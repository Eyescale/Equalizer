
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "layout.h"

#include "config.h"
#include "global.h"
#include "layoutVisitor.h"
#include "nodeFactory.h"
#include "view.h"

using namespace eq::base;

namespace eq
{

Layout::Layout()
        : _config( 0 )
{
}

Layout::~Layout()
{
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        EQCHECK( _removeView( view ));
        delete view;
    }

    _views.clear();
    EQASSERT( !_config );
}

void Layout::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_ALL ) // children are immutable
    {
        EQASSERT( _views.empty( ));
        EQASSERT( _config );

        NodeFactory* nodeFactory = Global::getNodeFactory();
        uint32_t id;
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            View* view = nodeFactory->createView();
            view->_layout = this;
            _views.push_back( view );

            _config->mapObject( view, id );
            view->becomeMaster();
        }
    }
}

void Layout::deregister()
{
    EQASSERT( _config );
    EQASSERT( isMaster( ));

    NodeFactory* nodeFactory = Global::getNodeFactory();

    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        EQASSERT( view->getID() != EQ_ID_INVALID );
        EQASSERT( view->isMaster( ));

        _config->deregisterObject( view );
        view->_layout = 0;
        nodeFactory->releaseView( view );
    }

    _views.clear();
    _config->deregisterObject( this );
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

void Layout::_addView( View* view )
{
    EQASSERT( view );
    view->_layout = this;
    _views.push_back( view );
}

bool Layout::_removeView( View* view )
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
