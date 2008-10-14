
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/client/wall.h>

using namespace eq;
using namespace std;

// Tests the wall description

int main( int argc, char **argv )
{
    Wall wall;
    wall.bottomLeft  = vmml::Vector3f( -.5f, -.5f, -.5f );
    wall.bottomRight = vmml::Vector3f( 1.f,  1.f,  -.5f );
    wall.topLeft     = vmml::Vector3f( -.5f, -.5f,  .5f );

    Wall target;
    target.bottomLeft  = vmml::Vector3f( -.875f,  -.875f, -.5f );
    target.bottomRight = vmml::Vector3f( 1.375f,  1.375f, -.5f );
    target.topLeft     = vmml::Vector3f( -.875f,  -.875f,  .5f );
    
    Wall tmp( wall );
    tmp.resizeHorizontal( 1.5f );
    TESTINFO( tmp == target, tmp << " != " << target );

    target.bottomLeft  = vmml::Vector3f( -.125f, -.125f, -.5f );
    target.bottomRight = vmml::Vector3f(  .625f,  .625f, -.5f );
    target.topLeft     = vmml::Vector3f( -.125f, -.125f,  .5f );
    
    tmp = wall;
    tmp.resizeHorizontal( .5f );
    TESTINFO( tmp == target, tmp << " != " << target );


    target.bottomLeft  = vmml::Vector3f( -.5f, -.5f, -.25f );
    target.bottomRight = vmml::Vector3f(  1.f, 1.f,  -.25f );
    target.topLeft     = vmml::Vector3f( -.5f, -.5f,  .25f );
    
    tmp = wall;
    tmp.resizeVertical( .5f );
    TESTINFO( tmp == target, tmp << " != " << target );
}
