
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

namespace eqPly
{

FrameData::FrameData()
        : _modelID( EQ_ID_INVALID )
        , _renderMode( mesh::RENDER_MODE_DISPLAY_LIST )
        , _color( true )
        , _ortho( false )
        , _statistics( false )
        , _wireframe( false )
        , _currentViewID( EQ_ID_INVALID )
{
    reset();
    EQINFO << "New FrameData " << std::endl;
}

void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        os << _translation << _rotation;
    if( dirtyBits & DIRTY_FLAGS )
        os << _modelID << _renderMode << _color << _ortho << _statistics
           << _wireframe;
    if( dirtyBits & DIRTY_VIEW )
        os << _currentViewID;
}

void FrameData::deserialize( eq::net::DataIStream& is,
                             const uint64_t dirtyBits )
{
    eq::Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_CAMERA )
        is >> _translation >> _rotation;
    if( dirtyBits & DIRTY_FLAGS )
        is >> _modelID >> _renderMode >> _color >> _ortho >> _statistics
           >> _wireframe;
    if( dirtyBits & DIRTY_VIEW )
        is >> _currentViewID;
}

void FrameData::setModelID( const uint32_t id )
{
    _modelID = id;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setColor( const bool onOff )
{
    _color = onOff;
    setDirty( DIRTY_FLAGS );
}

void FrameData::setRenderMode( const mesh::RenderMode mode )
{
    _renderMode = mode;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleOrtho()
{
    _ortho = !_ortho;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleStatistics()
{
    _statistics = !_statistics;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleWireframe()
{
    _wireframe = !_wireframe;
    setDirty( DIRTY_FLAGS );
}

void FrameData::toggleRenderMode()
{
    _renderMode = static_cast< mesh::RenderMode >(
        ( _renderMode + 1) % mesh::RENDER_MODE_ALL );

    EQINFO << "Switched to " << _renderMode << std::endl;
    setDirty( DIRTY_FLAGS );
}

void FrameData::spinCamera( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    _rotation.preRotateX( x );
    _rotation.preRotateY( y );
    setDirty( DIRTY_CAMERA );
}

void FrameData::moveCamera( const float x, const float y, const float z )
{
    _translation.x += x;
    _translation.y += y;
    _translation.z += z;

    setDirty( DIRTY_CAMERA );
}

void FrameData::reset()
{
    _translation   = vmml::Vector3f::ZERO;
    _translation.z = -2.f;
    _rotation = vmml::Matrix4f::IDENTITY;
    _rotation.rotateX( static_cast<float>( -M_PI_2 ));
    _rotation.rotateY( static_cast<float>( -M_PI_2 ));
    setDirty( DIRTY_CAMERA );
}

void FrameData::setCurrentViewID( const uint32_t id )
{
    _currentViewID = id;
    setDirty( DIRTY_VIEW );
}

}

