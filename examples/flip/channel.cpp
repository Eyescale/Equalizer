
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"
#include "frameData.h"

using namespace std;

bool Channel::init( const uint32_t initID )
{
    cout << "Init channel initID " << initID << " ptr " << this << endl;
    _frameData = (FrameData*)getConfig()->getMobject( initID );
    EQASSERT( _frameData );
    return true;
}

void Channel::exit()
{
    cout << "Exit " << this << endl;
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
    
    _frameData->sync( frameID );
    
    glTranslatef( 0, 0, -3 );
    glRotatef( _frameData->spin, 0, 0, 1. );
    
    glColor3f( 1, 1, 0 );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -.25, -.25, -.25 );
    glVertex3f( -.25,  .25, -.25 );
    glVertex3f(  .25, -.25, -.25 );
    glVertex3f(  .25,  .25, -.25 );
    glEnd();
    glFinish();
}

