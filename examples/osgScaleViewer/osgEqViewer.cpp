
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifdef WIN32
#define EQ_IGNORE_GLEW 
#endif

#include "osgEqViewer.h"

OSGEqViewer::OSGEqViewer()
        : _context( new osgViewer::GraphicsWindowEmbedded( ))
{
    setThreadingModel( osgViewer::Viewer::ThreadPerContext );
    setUpThreading();

    getCamera()->setClearMask( 0 ); // done by Channel::frameClear()
    _context->setClearMask( 0 );

    getCamera()->setGraphicsContext( _context.get( )); 
}

void OSGEqViewer::setViewport( eq::Channel* channel )
{
    //Set the Frustum of the Channel
    const eq::Frustumf& channelfrustum = channel->getFrustum();
    getCamera()->setProjectionMatrixAsFrustum( 
                    channelfrustum.left(), channelfrustum.right(),
                    channelfrustum.bottom(), channelfrustum.top(),
                    channelfrustum.near_plane(), channelfrustum.far_plane( ));

    //Set absolute pixelvalues for Viewport
    const eq::PixelViewport& pvp = channel->getPixelViewport();
    getCamera()->setViewport( pvp.x, pvp.y, pvp.w,pvp.h );
}
