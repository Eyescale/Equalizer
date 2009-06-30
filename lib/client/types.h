
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/base/refPtr.h>

#include <vmmlib/vmmlib.hpp>

#include <map>
#include <vector>

struct EqCompressorInfo;

namespace eq
{

class Canvas;
class Channel;
class Client;
class Config;
class Compressor;
class Frame;
class Image;
class Layout;
class Node;
class Observer;
class Pipe;
class PixelViewport;
class Segment;
class Server;
class Texture;
class View;
class Viewport;
class Window;
class X11Connection;
struct Statistic;

//----- Vectors
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;
typedef std::vector< Compressor* >   CompressorVector;
typedef std::vector< Frame* >    FrameVector;
typedef std::vector< Image* >    ImageVector;
typedef std::vector< Texture* >  TextureVector;

typedef std::vector< Observer* > ObserverVector;
typedef std::vector< Canvas* >   CanvasVector;
typedef std::vector< Layout* >   LayoutVector;
typedef std::vector< Segment* >  SegmentVector;
typedef std::vector< View* >     ViewVector;
typedef std::vector< std::string >   StringVector;

typedef std::vector< Viewport >      ViewportVector;
typedef std::vector< PixelViewport > PixelViewportVector;


//----- Reference Pointers
typedef base::RefPtr< X11Connection > X11ConnectionPtr;
typedef base::RefPtr< Client >        ClientPtr;
typedef base::RefPtr< Server >        ServerPtr;

//----- Others
typedef std::vector< Statistic >         Statistics;
typedef std::vector< EqCompressorInfo >  CompressorInfoVector;

// originator id -> statistics
typedef std::map< uint32_t, Statistics >        SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;


// vmml types definition 
typedef vmml::matrix< 3, 3, double > Matrix3d;
typedef vmml::matrix< 4, 4, double > Matrix4d;
typedef vmml::matrix< 4, 4, float >  Matrix4f;
typedef vmml::matrix< 3, 3, float >  Matrix3f;
typedef vmml::vector< 2, int > Vector2i;
typedef vmml::vector< 3, int > Vector3i;
typedef vmml::vector< 4, int > Vector4i;
typedef vmml::vector< 4, double >Vector4d;
typedef vmml::vector< 3, double >Vector3d;
typedef vmml::vector< 2, float > Vector2f;
typedef vmml::vector< 3, float > Vector3f;
typedef vmml::vector< 4, float > Vector4f;
typedef vmml::vector< 3, unsigned char > Vector3ub;
typedef vmml::frustum< float >  Frustumf;
//typedef vmml::frustum< double > Frustumd;
typedef vmml::frustum_culler< float >  FrustumCullerf;
//typedef vmml::frustum_culler< double > FrustumCullerd;

}
#endif // EQ_TYPES_H
