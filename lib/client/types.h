
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
/** A vector of pointers to eq::Node */
typedef std::vector< Node* >     NodeVector;
/** A vector of pointers to eq::Pipe */
typedef std::vector< Pipe* >     PipeVector;
/** A vector of pointers to eq::Window */
typedef std::vector< Window* >   WindowVector;
/** A vector of pointers to eq::Channel */
typedef std::vector< Channel* >  ChannelVector;
/** A vector of pointers to eq::Frame */
typedef std::vector< Frame* >    FrameVector;
/** A vector of pointers to eq::Image */
typedef std::vector< Image* >    ImageVector;
/** A vector of pointers to eq::Texture */
typedef std::vector< Texture* >  TextureVector;
/** A vector of pointers to eq::Observer */
typedef std::vector< Observer* > ObserverVector;
/** A vector of pointers to eq::Canvas */
typedef std::vector< Canvas* >   CanvasVector;
/** A vector of pointers to eq::Layout */
typedef std::vector< Layout* >   LayoutVector;
/** A vector of pointers to eq::Segment */
typedef std::vector< Segment* >  SegmentVector;
/** A vector of pointers to eq::View */
typedef std::vector< View* >     ViewVector;
/** A vector of eq::Viewport */
typedef std::vector< Viewport >      ViewportVector;
/** A vector of eq::PixelViewport */
typedef std::vector< PixelViewport > PixelViewportVector;
/** A vector of eq::Statistic events */
typedef std::vector< Statistic >         Statistics;

/** A reference-counted pointer to an eq::Client */
typedef base::RefPtr< Client >        ClientPtr;
/** A reference-counted pointer to an eq::Server */
typedef base::RefPtr< Server >        ServerPtr;

typedef vmml::matrix< 3, 3, double > Matrix3d; //!< A 3x3 double matrix
typedef vmml::matrix< 4, 4, double > Matrix4d; //!< A 4x4 double matrix
typedef vmml::matrix< 3, 3, float >  Matrix3f; //!< A 3x3 float matrix
typedef vmml::matrix< 4, 4, float >  Matrix4f; //!< A 4x4 float matrix
typedef vmml::vector< 2, int > Vector2i; //!< A two-component integer vector
typedef vmml::vector< 3, int > Vector3i; //!< A three-component integer vector
typedef vmml::vector< 4, int > Vector4i; //!< A four-component integer vector
typedef vmml::vector< 3, double >Vector3d; //!< A three-component double vector
typedef vmml::vector< 4, double >Vector4d; //!< A four-component double vector
typedef vmml::vector< 2, float > Vector2f; //!< A four-component float vector
typedef vmml::vector< 3, float > Vector3f; //!< A four-component float vector
typedef vmml::vector< 4, float > Vector4f; //!< A four-component float vector
/** A three-component byte vector */
typedef vmml::vector< 3, unsigned char > Vector3ub;
typedef vmml::frustum< float >  Frustumf; //!< A frustum definition
/** Frustum culling helper */
typedef vmml::frustum_culler< float >  FrustumCullerf;

/** A vector of std::string */
typedef std::vector< std::string >   StringVector;
/** A vector of bytes */
typedef std::vector<uint8_t>    UByteVector;
/** A vector of unsigned shorts */
typedef std::vector<uint16_t>   UShortVector;


/** @cond IGNORE */
typedef base::RefPtr< X11Connection > X11ConnectionPtr;
typedef std::vector< EqCompressorInfo >  CompressorInfoVector;
typedef std::vector< Compressor* >   CompressorVector;

// originator id -> statistics
typedef std::map< uint32_t, Statistics >        SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;
/** @endcond */
}
#endif // EQ_TYPES_H
