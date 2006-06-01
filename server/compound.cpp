
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"

#include <eq/base/base.h>
#include <eq/client/packets.h>
#include <eq/client/wall.h>
#include <eq/net/session.h>

#include <GL/gl.h>
#include <algorithm>
#include <math.h>
#include <vector>

using namespace eqs;
using namespace eqBase;
using namespace std;

Compound::Compound()
        : _parent( NULL )
{
}

// copy constructor
Compound::Compound( const Compound& from )
        : _parent( NULL )
{
    const uint32_t nChildren = from.nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
    {
        const Compound* child = from.getChild(i);
        addChild( new Compound( *child ));
    }

    _view = from._view;
    _data = from._data;
    _mode = from._mode;
}

Compound::InheritData::InheritData()
        : channel( NULL )
{}

void Compound::addChild( Compound* child )
{
    _children.push_back( child );
    EQASSERT( !child->_parent );
    child->_parent = this;
}

Compound* Compound::_getNext() const
{
    if( !_parent )
        return NULL;

    vector<Compound*>&          siblings = _parent->_children;
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
// view operations
//---------------------------------------------------------------------------
void Compound::setWall( const eq::Wall& wall )
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
    _data.view.width = length;

    length = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
    _data.view.height = length;

    length = sqrt( w[0]*w[0] + w[1]*w[1] + w[2]*w[2] );
    w[0] /= length;
    w[1] /= length;
    w[2] /= length;

    _data.view.xfm[0]  = u[0];
    _data.view.xfm[1]  = v[0];
    _data.view.xfm[2]  = w[0];
    _data.view.xfm[3]  = 0.;
                        
    _data.view.xfm[4]  = u[1];
    _data.view.xfm[5]  = v[1];
    _data.view.xfm[6]  = w[1];
    _data.view.xfm[7]  = 0.;
                        
    _data.view.xfm[8]  = u[2];
    _data.view.xfm[9]  = v[2];
    _data.view.xfm[10] = w[2];
    _data.view.xfm[11] = 0.;

    const float center[3] = { (wall.bottomRight[0] + wall.topLeft[0]) / 2.,
                              (wall.bottomRight[1] + wall.topLeft[1]) / 2.,
                              (wall.bottomRight[2] + wall.topLeft[2]) / 2. };

    _data.view.xfm[12] = -(u[0]*center[0] + u[1]*center[1] + u[2]*center[2]);
    _data.view.xfm[13] = -(v[0]*center[0] + v[1]*center[1] + v[2]*center[2]);
    _data.view.xfm[14] = -(w[0]*center[0] + w[1]*center[1] + w[2]*center[2]);
    _data.view.xfm[15] = 1.;

    EQVERB << "Wall matrix: " << LOG_MATRIX4x4( _data.view.xfm ) << endl;

    _view.wall        = wall;
    _view.latest      = View::WALL;
}

#define DEG2RAD( angle ) ( (angle) * M_PI / 180.f )

void Compound::setProjection( const eq::Projection& projection )
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
    const float* origin   = projection.origin;
    const float  distance = projection.distance;
    const float  trans[3] = 
        {
            -( rot[0]*origin[0] + rot[3]*origin[1] + rot[6]*origin[2] ),
            -( rot[1]*origin[0] + rot[4]*origin[1] + rot[7]*origin[2] ),
            -( rot[2]*origin[0] + rot[5]*origin[1] + rot[8]*origin[2] )
        };

    _data.view.xfm[0]  = rot[0];
    _data.view.xfm[1]  = rot[1];
    _data.view.xfm[2]  = rot[2];
    _data.view.xfm[3]  = 0.;

    _data.view.xfm[4]  = rot[3];
    _data.view.xfm[5]  = rot[4];
    _data.view.xfm[6]  = rot[5];
    _data.view.xfm[7]  = 0.;
                       
    _data.view.xfm[8]  = rot[6];                
    _data.view.xfm[9]  = rot[7];
    _data.view.xfm[10] = rot[8];
    _data.view.xfm[11] = 0.;

    _data.view.xfm[12] = trans[0];
    _data.view.xfm[13] = trans[1];
    _data.view.xfm[14] = trans[2] + distance;
    _data.view.xfm[15] = 1.;

    _data.view.width  = distance * tan(DEG2RAD( projection.fov[0] ));
    _data.view.height = distance * tan(DEG2RAD( projection.fov[1] ));

    _view.projection = projection;
    _view.latest     = View::PROJECTION;
}

void Compound::setViewMatrix( const eq::ViewMatrix& view )
{
    _data.view   = view;
    _view.matrix = view;
    _view.latest = View::VIEWMATRIX;
}

//---------------------------------------------------------------------------
// traverse
//---------------------------------------------------------------------------
TraverseResult Compound::traverse( Compound* compound, TraverseCB preCB,
                                   TraverseCB leafCB, TraverseCB postCB,
                                   void *userData )
{
    if ( compound->isLeaf( )) 
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
    _updateInheritData();
    _updateSwapGroup();

    const uint32_t nChildren = this->nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
    {
        Compound* child = getChild(i);
        child->update();
    }
}

void Compound::_updateInheritData()
{
    if( !_parent )
    {
        _inherit = _data;
        return;
    }

    _inherit = _parent->_inherit;

    if( !_inherit.channel ) 
        _inherit.channel = _data.channel;

    if( !_inherit.view.isValid( ))
        _inherit.view = _data.view;

    _inherit.vp.multiply( _data.vp );
}

void Compound::_updateSwapGroup()
{
    const bool sync = ( _mode == MODE_SYNC );

    // synchronize swap of all children
    Window*        master    = NULL;
    const uint32_t nChildren = this->nChildren();
    for( uint32_t i=0; i<nChildren; i++ )
    {
        const Compound* child  = getChild(i);
        Window*         window = child->getWindow();
        
        if( !window ) continue;

        if( !master )
            master = window;

        window->resetSwapGroup();
        if( sync )
            window->setSwapGroup( master );
    }
}

struct UpdateChannelData
{
    Channel* channel;
    uint32_t frameID;
};

void Compound::updateChannel( Channel* channel, const uint32_t frameID )
{
    UpdateChannelData data = { channel, frameID };
    traverse( this, NULL, _updateDrawCB, _updatePostDrawCB, &data );
}

TraverseResult Compound::_updateDrawCB( Compound* compound, void* userData )
{
    UpdateChannelData* data    = (UpdateChannelData*)userData;
    Channel*           channel = data->channel;

    if( compound->_data.channel != channel )
        return TRAVERSE_CONTINUE;

    const Channel*           iChannel = compound->_inherit.channel;
    const eq::PixelViewport& pvp      = iChannel->getPixelViewport();

    eq::ChannelDrawPacket drawPacket( channel->getSession()->getID(), 
                                      channel->getID( ));

    drawPacket.context.hints      = HINT_BUFFER;
    drawPacket.context.drawBuffer = GL_BACK;
    drawPacket.context.vp         = compound->_inherit.vp;
    drawPacket.context.pvp.x      = 0;
    drawPacket.context.pvp.y      = 0;
    drawPacket.context.pvp.w      = (uint32_t)(compound->_inherit.vp.w * pvp.w);
    drawPacket.context.pvp.h      = (uint32_t)(compound->_inherit.vp.h * pvp.h);
    drawPacket.frameID            = data->frameID;

    if( !compound->_parent || 
        compound->_parent && compound->_parent->_data.channel != channel )
    {
        eq::ChannelClearPacket clearPacket( channel->getSession()->getID(), 
                                            channel->getID( ));
        
        clearPacket.context.hints      = HINT_BUFFER;
        clearPacket.context.drawBuffer = drawPacket.context.drawBuffer;
        clearPacket.context.vp         = drawPacket.context.vp;
        clearPacket.context.pvp        = drawPacket.context.pvp;
        clearPacket.frameID            = data->frameID;
        channel->send( clearPacket );
    }

    drawPacket.context.hints |= HINT_FRUSTUM;
    compound->_computeFrustum( drawPacket.context.frustum, 
                     drawPacket.context.headTransform );

    channel->send( drawPacket );
    return TRAVERSE_CONTINUE;
}

void Compound::_computeFrustum( eq::Frustum& frustum, float headTransform[16] )
{
    const Channel*        iChannel = _inherit.channel;
    const eq::ViewMatrix& iView    = _inherit.view;

    iChannel->getNearFar( &frustum.near, &frustum.far );

    // eye position in screen space
    const float  head[3] = { 0, 0, 0 }; // TODO get from headtracking API
    const float* xfm     = iView.xfm;
    const float  w       = 
        xfm[3] * head[0] + xfm[7] * head[1] + xfm[11]* head[2] + xfm[15];
    const float  eye[3]  = {
        (xfm[0] * head[0] + xfm[4] * head[1] + xfm[8] * head[2] + xfm[12]) / w,
        (xfm[1] * head[0] + xfm[5] * head[1] + xfm[9] * head[2] + xfm[13]) / w,
        (xfm[2] * head[0] + xfm[6] * head[1] + xfm[10]* head[2] + xfm[14]) / w};
    
    // compute frustum from size and eye position
    const float ratio = frustum.near / eye[2];
    if( eye[2] > 0 )
    {
        frustum.left   = -( iView.width/2.  + eye[0] ) * ratio;
        frustum.right  =  ( iView.width/2.  - eye[0] ) * ratio;
        frustum.top    = -( iView.height/2. + eye[1] ) * ratio;
        frustum.bottom =  ( iView.height/2. - eye[1] ) * ratio;
    }
    else // eye behind near plane - 'mirror' x
    {
        frustum.left   =  ( iView.width/2.  - eye[0] ) * ratio;
        frustum.right  = -( iView.width/2.  + eye[0] ) * ratio;
        frustum.top    =  ( iView.height/2. + eye[1] ) * ratio;
        frustum.bottom = -( iView.height/2. - eye[1] ) * ratio;
    }

    // compute head transform
    for( int i=0; i<16; i += 4 )
    {
        headTransform[i]   = xfm[i]   - eye[0] * xfm[i+3];
        headTransform[i+1] = xfm[i+1] - eye[1] * xfm[i+3];
        headTransform[i+2] = xfm[i+2] - eye[2] * xfm[i+3];
        headTransform[i+3] = xfm[i+3];
    }
}


TraverseResult Compound::_updatePostDrawCB( Compound* compound, void* userData )
{
//    UpdateChannelData* data = (UpdateChannelData*)userData;
//    if( compound->_data.channel != data->channel )
        return TRAVERSE_CONTINUE;
}

std::ostream& eqs::operator << (std::ostream& os, const Compound* compound)
{
    if( !compound )
        return os;
    
    os << "compound" << endl;
    os << "{" << endl << indent;
      
    const Compound::Mode mode = compound->getMode();
    if( mode != Compound::MODE_SYNC )
        os << "mode [" << ( mode == Compound::MODE_2D   ? "2D" : "???" ) << "]" 
           << endl;

    const Channel* channel = compound->getChannel();
    if( channel )
        os << "channel \"" << channel->getName() << "\"" << endl;

    switch( compound->_view.latest )
    {
        case Compound::View::WALL:
            os << compound->getWall() << endl;
            break;
        case Compound::View::PROJECTION:
            //os << compound->getProjection() << endl;
            break;
        case Compound::View::VIEWMATRIX:
            //os << compound->getViewMatrix() << endl;
            break;
        default: 
            break;
    }

    const uint32_t nChildren = compound->nChildren();
    if( nChildren > 0 )
    {
        os << endl;
        for( uint32_t i=0; i<nChildren; i++ )
            os << compound->getChild(i);
    }

    os << exdent << "}" << endl;
    return os;
}
