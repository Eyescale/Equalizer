
/* Copyright (c)
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

#ifndef OSGSV_PIPE_H
#define OSGSV_PIPE_H

#ifdef WIN32
#  define EQ_IGNORE_GLEW
#endif

#include <eq/eq.h>

#include "osgEqViewer.h"
#include "frameData.h"

#include <osg/MatrixTransform>
#include <osg/Matrix>
#include <osg/ref_ptr>
#include <osg/Node>
#include <osg/Image>

namespace osgScaleViewer
{
    /**
     * The Pipe holds the viewer and the frame data.
     * Each frame, it updates the scene graph of the viewer with the
     * new data of the frame data. The frame data is synced with the server.
     */
    class Pipe : public eq::Pipe
    {
    public:
        /** 
         * Creates a Pipe.
         * @param parent the pipe's parent.
         */
        Pipe( eq::Node* parent );
   
        /** 
         * Gets the FrameData object.
         * @return the frame data object.
         */
        const FrameData& getFrameData() const;

    protected:
        virtual ~Pipe();

        /**
         * Creates the scene graph and registers the frame data, so it can be
         * synced with the server later.
         */
        virtual bool configInit( const uint32_t initID );

        /**
         * Deregisters the frame data.
         */
        virtual bool configExit();

        /**
         * Syncs the frame data with the server and calls updateSceneGraph().
         */
        virtual void frameStart( const uint32_t frameID,
                                 const uint32_t frameNumber );

    private:
        FrameData _frameData;
    };
}

#endif
