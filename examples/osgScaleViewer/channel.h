
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef WIN32
#define EQ_IGNORE_GLEW
#endif

#include <eq/eq.h>

#include <osg/LightSource>
#include <osg/Matrix>
#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

namespace osgScaleViewer
{

class FrameData;
class Pipe;

/**
 * The Channel renders the frames in frameDraw().
 * This is done by querying the pipe for the viewer and then asking the viewer
 * to render the scene.
 */
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );

protected:
    /** @sa eq::Channel::frameClear() */
    virtual void frameClear( const uint32_t frameID );

    /** @sa eq::Channel::frameDraw() **/
    virtual void frameDraw( const uint32_t frameID );

    /** @sa eq::Channel::frameViewFinish() */
    void frameViewFinish( const uint32_t frameID );

private:
};

}
#endif
