
/* Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@eyescale.ch>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSGSV_PIPE_H
#define OSGSV_PIPE_H

typedef void* HPBUFFERARB;
#include <eq/eq.h>

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
        virtual bool configInit( const eq::uint128_t& initID );

        /**
         * Deregisters the frame data.
         */
        virtual bool configExit();

        /**
         * Syncs the frame data with the server and calls updateSceneGraph().
         */
        virtual void frameStart( const eq::uint128_t& frameID,
                                 const uint32_t frameNumber );

    private:
        FrameData _frameData;
    };
}

#endif
