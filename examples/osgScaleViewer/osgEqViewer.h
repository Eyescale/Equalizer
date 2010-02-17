
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

#ifndef OSGEQVIEWER_H
#define OSGEQVIEWER_H

#include <eq/eq.h>
#include <osgViewer/Viewer>

/**
 * An OpenSceneGraph viewer which can be used with Equalizer.
 *
 * A normal Viewer from OSG would try to create its own windows, which
 * we don't want to, since they are set up by Equalizer for us.
 *
 * The OSGEqViewer solves this by setting the correct context and viewport.
 *
 * Call setViewport() in your frameDraw() method.
 */
class OSGEqViewer : public osgViewer::Viewer
{
public:
    /** 
     * Constructs the OpenSceneGraph viewer.
     */
    OSGEqViewer();

    /**
     * Sets the viewport of the Camara of the scene so that it matches
     * the viewport of the given Equalizer channel.
     */
    void setViewport( eq::Channel* channel );

private:
    /** The OSG Viewer context. */
    osg::ref_ptr< osgViewer::GraphicsWindowEmbedded > _context;
};

#endif
