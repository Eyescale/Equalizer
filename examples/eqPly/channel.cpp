
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"

using namespace std;
using namespace eqBase;

static float lightpos[] = { 0., 0., 1., 0. };

bool Channel::init( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;
    eq::Config* config = getConfig();

    _initData  = (InitData*)config->getObject( initID );
    _frameData = _initData->getFrameData();
    
    return true;
}

bool Channel::exit()
{
    _initData  = NULL;
    _frameData = NULL;
    EQINFO << "Exit " << this << endl;
    return eq::Channel::exit();
}

void Channel::draw( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyFrustum();
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    glLightfv( GL_LIGHT0, GL_POSITION, lightpos );

    glTranslatef( _frameData->_data.translation[0],
                  _frameData->_data.translation[1],
                  _frameData->_data.translation[2] );
    glMultMatrixf( _frameData->_data.rotation );


    Node*        node  = (Node*)getNode();
    const Model* model = node->getModel();

    if( model )
    {
        _drawBBox( model->getBBox( ));
    }
    else
    {
        glColor3f( 1, 1, 0 );
        glBegin( GL_TRIANGLE_STRIP );
        glVertex3f( -.25, -.25, -.25 );
        glVertex3f( -.25,  .25, -.25 );
        glVertex3f(  .25, -.25, -.25 );
        glVertex3f(  .25,  .25, -.25 );
        glEnd();
        glFinish();
    }
}

void Channel::_drawBBox( const Model::BBox *bbox )
{
    const size_t nFaces = bbox->nFaces;
            
    glBegin( GL_TRIANGLES );    
    for( size_t i=0; i<nFaces; i++ )
    {
        const NormalFace<ColorVertex> &face = bbox->faces[i];
        
        glColor3fv(  face.vertices[0].color );
        glNormal3fv( face.normal );
        glVertex3fv( face.vertices[0].pos );
        
        glColor3fv(  face.vertices[1].color );
        glNormal3fv( face.normal ); 
        glVertex3fv( face.vertices[1].pos );
        
        glColor3fv(  face.vertices[2].color );
        glNormal3fv( face.normal ); 
        glVertex3fv( face.vertices[2].pos );
    }
    glEnd();
}

