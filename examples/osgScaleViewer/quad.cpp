
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

#include "quad.h"

#include <osg/Geode>
#include <osg/Geometry>

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
    data->push_back( osg::Vec3( -widthf/2.f, 0.f, -heightf/2.f ));
    data->push_back( osg::Vec3( -widthf/2.f, 0.f,  heightf/2.f ));
    data->push_back( osg::Vec3(  widthf/2.f, 0.f,  heightf/2.f ));
    data->push_back( osg::Vec3(  widthf/2.f, 0.f, -heightf/2.f ));

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
    data->push_back( osg::Vec3( -1.f, 0.f, -1.f ));
    data->push_back( osg::Vec3( -1.f, 0.f,  1.f ));
    data->push_back( osg::Vec3(  1.f, 0.f,  1.f ));
    data->push_back( osg::Vec3(  1.f, 0.f, -1.f ));

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
