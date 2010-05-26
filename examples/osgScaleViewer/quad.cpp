
/*
 * Copyright (c)
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

#include "quad.h"

#include <osg/Geode>
#include <osg/Geometry>

namespace osgScaleViewer
{
osg::ref_ptr<osg::Node> Quad::createQuad() const
{
    osg::ref_ptr<osg::Geode> geode= new osg::Geode();

    geode->addDrawable( createDrawable().get( ));
    geode->setDataVariance( osg::Object::STATIC );

    return geode.get();
}

osg::ref_ptr<osg::Node> Quad::createQuad( int width, int height ) const
{
    osg::ref_ptr<osg::Geode> geode= new osg::Geode();

    geode->addDrawable( createDrawable( width, height ).get( ));
    geode->setDataVariance( osg::Object::STATIC );

    return geode.get();
}

osg::ref_ptr<osg::Drawable> Quad::createDrawable( int width, int height ) const
{
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

    // vertices
    float heightf = 5.f;
    float widthf =  heightf * width / static_cast<float>( height );
    
    osg::ref_ptr<osg::Vec3Array> data = new osg::Vec3Array();
    data->push_back( osg::Vec3(  widthf/2.f, -heightf/2.f, 0.f ));
    data->push_back( osg::Vec3(  widthf/2.f,  heightf/2.f, 0.f ));
    data->push_back( osg::Vec3( -widthf/2.f,  heightf/2.f, 0.f ));
    data->push_back( osg::Vec3( -widthf/2.f, -heightf/2.f, 0.f ));

    // color
    osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1.f, 1.f, 1.f, 1.f ));

    // normals
    osg::ref_ptr<osg::Vec3Array> normals= new osg::Vec3Array();
    normals->push_back( osg::Vec3( 0.f, -1.f, 0.f ));

    // texture coordinates
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set( 1.f, 0.f );
    (*texcoords)[1].set( 1.f, 1.f );
    (*texcoords)[2].set( 0.f, 1.f );
    (*texcoords)[3].set( 0.f, 0.f );

    // assign data
    geom->setVertexArray( data.get( ));
    geom->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, 4 ));

    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->setColorArray( color.get( ));

    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->setNormalArray( normals.get( ));

    geom->setTexCoordArray( 0, texcoords );
    geom->setDataVariance( osg::Object::STATIC );

    return geom.get();
}

osg::ref_ptr<osg::Drawable> Quad::createDrawable() const
{
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

    // vertices
    osg::ref_ptr<osg::Vec3Array> data = new osg::Vec3Array();
    data->push_back( osg::Vec3(  1.f, -1.f, 0.f ));
    data->push_back( osg::Vec3(  1.f,  1.f, 0.f ));
    data->push_back( osg::Vec3( -1.f,  1.f, 0.f ));
    data->push_back( osg::Vec3( -1.f, -1.f, 0.f ));

    // color
    osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1.f, 0.f, 0.f, 0.f ));

    // normals
    osg::ref_ptr<osg::Vec3Array> normals= new osg::Vec3Array();
    normals->push_back( osg::Vec3( 0.f, -1.f, 0.f ));

    // assign data
    geom->setVertexArray( data.get( ));
    geom->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, 4 ));

    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    geom->setColorArray( color.get( ));

    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->setNormalArray( normals.get( ));

    return geom.get();
}

}
