
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"

#include <eq/base/base.h>
#include <algorithm>
#include <vector>

using namespace eqs;
using namespace std;

Compound::Compound()
        : _parent( NULL ),
          _channel( NULL )
{}

void Compound::addChild( Compound* child )
{
    _children.push_back( child );
    ASSERT( !child->_parent );
    child->_parent = this;
}

Compound* Compound::_getNext() const
{
    if( !_parent )
        return NULL;

    vector<Compound*>           siblings = _parent->_children;
    vector<Compound*>::iterator result   = find( siblings.begin(),
                                                 siblings.end(), this);

    if( result == siblings.end() )
        return NULL;
    result++;
    if( result == siblings.end() )
        return NULL;

    return *result;
}

//---------------------------------------------------------------------------
// traverse
//---------------------------------------------------------------------------
TraverseResult Compound::traverse( Compound* compound, TraverseCB preCB,
                                   TraverseCB leafCB, TraverseCB postCB,
                                   void *userData )
{
    if ( compound->nChildren( )) 
    {
        if ( leafCB ) 
            return leafCB( compound, userData );
        return TRAVERSE_CONTINUE;
    }

    Compound *current = compound;
    while( true )
    {
        Compound *parent = current->getParent();
        Compound *next   = current->_getNext();
        Compound *child  = (current->nChildren()) ? current->getChild(0) : NULL;

        //---------- down-right traversal
        if ( !child ) // leaf
        {
            if ( leafCB )
            {
                TraverseResult result = leafCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return result;
            }

            current = next;
        } 
        else // node
        {
            if( preCB )
            {
                TraverseResult result = preCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return result;
            }

            current = child;
        }

        //---------- up-right traversal
        if( current == NULL && parent == NULL ) return TRAVERSE_CONTINUE;

        while ( current == NULL )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->_getNext();

            if( postCB )
            {
                TraverseResult result = postCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return result;
            }
            
            if ( current == compound ) return TRAVERSE_CONTINUE;
            
            current = next;
        }
    }
    return TRAVERSE_CONTINUE;
}

void Compound::init()
{
    const uint nChildren = this->nChildren();
    for( uint i=0; i<nChildren; i++ )
    {
        Compound* child = getChild(i);
        child->init();
    }

    Channel* channel = getChannel();
    if( channel )
        channel->refUsed();
}

void Compound::exit()
{
    const uint nChildren = this->nChildren();
    for( uint i=0; i<nChildren; i++ )
    {
        Compound* child = getChild(i);
        child->exit();
    }

    Channel* channel = getChannel();
    if( channel )
        channel->unrefUsed();
}

std::ostream& eqs::operator << (std::ostream& os,const Compound* compound)
{
    if( !compound )
    {
        os << "NULL compound";
        return os;
    }
    
    const uint nChildren = compound->nChildren();
    os << "compound " << (void*)compound << " channel " 
       << compound->getChannel() << " " << nChildren << " children";
    
    for( uint i=0; i<nChildren; i++ )
        os << std::endl << "    " << compound->getChild(i);
    
    return os;
}
