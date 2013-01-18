
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef NODE_H
#define NODE_H

#include <eq/eq.h>

#include <osg/Image>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Node>

namespace osgScaleViewer
{
    class Node : public eq::Node
    {
    public:
        /** 
         * Creates a Node.
         * @param parent the node's parent.
         */
        Node( eq::Config* parent );

        int32_t getUniqueContextID() { return ++_contextID; }
        osg::ref_ptr< osg::Node > getModel() { return _model; }
        osg::ref_ptr< osg::FrameStamp > getFrameStamp() { return _frameStamp; }

    protected:
        virtual bool configInit( const eq::uint128_t& initID );
        virtual bool configExit();
        virtual void frameStart( const eq::uint128_t& frameID,
                                 const uint32_t frameNumber );

    private:
        lunchbox::a_int32_t _contextID;
        osg::ref_ptr< osg::Node > _model;
        osg::ref_ptr< osg::FrameStamp > _frameStamp;
        osg::ref_ptr< osg::NodeVisitor > _updateVisitor;

        osg::ref_ptr< osg::Node > _createSceneGraph();
        osg::ref_ptr< osg::Node > _createSceneGraph( osg::ref_ptr<osg::Image> );
        osg::ref_ptr< osg::Group > _initSceneGraph();
        osg::ref_ptr< osg::LightSource > _createLightSource();
    };

}
#endif
