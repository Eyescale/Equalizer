
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"

#include <eq/base/base.h>
#include <algorithm>
#include <math.h>
#include <vector>

using namespace eqs;
using namespace std;

Compound::Compound()
        : _parent( NULL ),
          _channel( NULL )
{
}

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
// frustum operations
//---------------------------------------------------------------------------
void Compound::setWall( const Wall& wall )
{
    float u[3] = { wall.bottomRight[0] - wall.bottomLeft[0],
                   wall.bottomRight[1] - wall.bottomLeft[1],
                   wall.bottomRight[2] - wall.bottomLeft[2] };

    float v[3] = { wall.topLeft[0] - wall.bottomLeft[0],
                   wall.topLeft[1] - wall.bottomLeft[1],
                   wall.topLeft[2] - wall.bottomLeft[2] };

    float w[3] = { u[1]*v[2] - u[2]*v[1],
                   u[2]*v[0] - u[0]*v[2],
                   u[0]*v[1] - u[1]*v[0] };

    float length = sqrt( u[0]*u[0] + u[1]*u[1] + u[2]*u[2] );
    u[0] /= length;
    u[1] /= length;
    u[2] /= length;
    _frustum.width = length;

    length = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
    _frustum.height = length;

    length = sqrt( w[0]*w[0] + w[1]*w[1] + w[2]*w[2] );
    w[0] /= length;
    w[1] /= length;
    w[2] /= length;

    _frustum.xfm[0]  = u[0];
    _frustum.xfm[1]  = v[0];
    _frustum.xfm[2]  = w[0];
    _frustum.xfm[3]  = 0.;
                        
    _frustum.xfm[4]  = u[1];
    _frustum.xfm[5]  = v[1];
    _frustum.xfm[6]  = w[1];
    _frustum.xfm[7]  = 0.;
                        
    _frustum.xfm[8]  = u[2];
    _frustum.xfm[9]  = v[2];
    _frustum.xfm[10] = w[2];
    _frustum.xfm[11] = 0.;

    const float center[3] = { (wall.bottomRight[0] + wall.topLeft[0]) / 2.,
                              (wall.bottomRight[1] + wall.topLeft[1]) / 2.,
                              (wall.bottomRight[2] + wall.topLeft[2]) / 2. };

    _frustum.xfm[12] = -( u[0]*center[0] + u[1]*center[1] + u[2]*center[2]);
    _frustum.xfm[13] = -( v[0]*center[0] + v[1]*center[1] + v[2]*center[2]);
    _frustum.xfm[14] = -( w[0]*center[0] + w[1]*center[1] + w[2]*center[2]);
    _frustum.xfm[15] = 1.;

    VERB << "Wall matrix: " << LOG_MATRIX4x4( _frustum.xfm ) << endl;

    _view.wall        = wall;
    _view.latest      = View::WALL;
}

#define DEG2RAD( angle ) ( (angle) * M_PI / 180.f )

void Compound::setProjection( const Projection& projection )
{
    const float cosH = cosf( DEG2RAD( projection.hpr[0] ));
    const float sinH = sinf( DEG2RAD( projection.hpr[0] ));
    const float cosP = cosf( DEG2RAD( projection.hpr[1] ));
    const float sinP = sinf( DEG2RAD( projection.hpr[1] ));
    const float cosR = cosf( DEG2RAD( projection.hpr[2] ));
    const float sinR = sinf( DEG2RAD( projection.hpr[2] ));

    // HPR Matrix = -roll[z-axis] * -pitch[x-axis] * -head[y-axis]
    const float rot[9] =
        {
            sinR*sinP*sinH + cosR*cosH,  cosR*sinP*sinH - sinR*cosH,  cosP*sinH,
            cosP*sinR,                   cosP*cosR,                  -sinP,
            sinR*sinP*cosH - cosR*sinH,  cosR*sinP*cosH + sinR*sinH,  cosP*cosH 
        };

    // translation = HPR x -origin
    const float *origin   = projection.origin;
    const float  trans[3] = 
        {
            -( rot[0]*origin[0] + rot[3]*origin[1] + rot[6]*origin[2] ),
            -( rot[1]*origin[0] + rot[4]*origin[1] + rot[7]*origin[2] ),
            -( rot[2]*origin[0] + rot[5]*origin[1] + rot[8]*origin[2] )
        };

    _frustum.xfm[0]  = rot[0];
    _frustum.xfm[1]  = rot[1];
    _frustum.xfm[2]  = rot[2];
    _frustum.xfm[3]  = 0.;

    _frustum.xfm[4]  = rot[3];
    _frustum.xfm[5]  = rot[4];
    _frustum.xfm[6]  = rot[5];
    _frustum.xfm[7]  = 0.;
                       
    _frustum.xfm[8]  = rot[6];                
    _frustum.xfm[9]  = rot[7];
    _frustum.xfm[10] = rot[8];
    _frustum.xfm[11] = 0.;

    _frustum.xfm[12] = trans[0];
    _frustum.xfm[13] = trans[1];
    _frustum.xfm[14] = trans[2] + projection.distance;
    _frustum.xfm[15] = 1.;

    _frustum.width  = projection.distance*tan( DEG2RAD( projection.fov[0] ));
    _frustum.height = projection.distance*tan( DEG2RAD( projection.fov[1] ));

    _view.projection = projection;
    _view.latest     = View::PROJECTION;
}

void Compound::setFrustum( const Frustum& frustum )
{
    _frustum      = frustum;
    _view.frustum = frustum;
    _view.latest  = View::FRUSTUM;
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
                    return TRAVERSE_TERMINATE;
            }

            current = next;
        } 
        else // node
        {
            if( preCB )
            {
                TraverseResult result = preCB( current, userData );
                if( result == TRAVERSE_TERMINATE )
                    return TRAVERSE_TERMINATE;
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
                    return TRAVERSE_TERMINATE;
            }
            
            if ( current == compound ) return TRAVERSE_CONTINUE;
            
            current = next;
        }
    }
    return TRAVERSE_CONTINUE;
}

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::init()
{
    const uint32_t nChildren = this->nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
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
    const uint32_t nChildren = this->nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
    {
        Compound* child = getChild(i);
        child->exit();
    }

    Channel* channel = getChannel();
    if( channel )
        channel->unrefUsed();
}

void Compound::update()
{
    const uint32_t nChildren = this->nChildren();

    if( nChildren == 0 ) // leaf
    {
    }

}


std::ostream& eqs::operator << (std::ostream& os,const Compound* compound)
{
    if( !compound )
    {
        os << "NULL compound";
        return os;
    }
    
    const uint32_t nChildren = compound->nChildren();
    os << "compound " << (void*)compound << " channel " 
       << compound->getChannel() << " " << nChildren << " children";
    
    for( uint32_t i=0; i<nChildren; i++ )
        os << std::endl << "    " << compound->getChild(i);
    
    return os;
}
