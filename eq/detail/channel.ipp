
/* Copyright (c) 2012-2015, Stefan Eilemann <eile@eyescale.ch>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "../channel.h"
#include "../image.h"
#include "../resultImageListener.h"
#include "fileFrameWriter.h"

#include <boost/foreach.hpp>

#ifdef EQUALIZER_USE_DEFLECT
#  include "../deflect/proxy.h"
#endif

namespace eq
{

namespace detail
{
enum State
{
    STATE_STOPPED,
    STATE_INITIALIZING,
    STATE_RUNNING,
    STATE_FAILED
};

class Channel
{
public:
    Channel()
        : state( STATE_STOPPED )
        , initialSize( Vector2i::ZERO )
#ifdef EQUALIZER_USE_DEFLECT
        , _deflectProxy( 0 )
#endif
        , _updateFrameBuffer( false )
    {
        lunchbox::RNG rng;
        color.r() = rng.get< uint8_t >();
        color.g() = rng.get< uint8_t >();
        color.b() = rng.get< uint8_t >();
    }

    ~Channel()
    {
        statistics->clear();
    }

    void addResultImageListener( ResultImageListener* listener )
    {
        LBASSERT( std::find( resultImageListeners.begin(),
                             resultImageListeners.end(), listener ) ==
                  resultImageListeners.end( ));

        resultImageListeners.push_back( listener );
    }

    void removeResultImageListener( ResultImageListener* listener )
    {
        ResultImageListeners::iterator i =
                std::find( resultImageListeners.begin(),
                           resultImageListeners.end(), listener );
        if( i != resultImageListeners.end( ))
            resultImageListeners.erase( i );
    }

    void frameViewFinish( eq::Channel& channel )
    {
        if( channel.getSAttribute( channel.SATTR_DUMP_IMAGE ).empty( ))
            removeResultImageListener( &frameWriter );
        else
        {
            ResultImageListeners::iterator i =
                    std::find( resultImageListeners.begin(),
                               resultImageListeners.end(), &frameWriter );
            if( i == resultImageListeners.end( ))
                addResultImageListener( &frameWriter );
        }

#ifdef EQUALIZER_USE_DEFLECT
        if( _deflectProxy && !_deflectProxy->isRunning( ))
        {
            delete _deflectProxy;
            _deflectProxy = 0;
        }
#endif

        if( resultImageListeners.empty( ))
            return;

        downloadFramebuffer( channel );
        BOOST_FOREACH( ResultImageListener* listener, resultImageListeners )
            listener->notifyNewImage( channel, framebufferImage );
    }

    void downloadFramebuffer( eq::Channel& channel )
    {
        framebufferImage.setAlphaUsage( true );
        framebufferImage.setQuality( eq::Frame::BUFFER_COLOR, 1.0f );
        framebufferImage.setStorageType( eq::Frame::TYPE_MEMORY );
        framebufferImage.setInternalFormat( eq::Frame::BUFFER_COLOR, GL_RGBA );

        if( framebufferImage.startReadback( eq::Frame::BUFFER_COLOR,
                                            channel.getPixelViewport(),
                                            channel.getZoom(),
                                            channel.getObjectManager( )))
        {
            framebufferImage.finishReadback( channel.glewGetContext( ));
        }
    }

    /** The configInit/configExit state. */
    State state;

    /** A random, unique color for this channel. */
    Vector3ub color;

    typedef std::vector< Statistic > Statistics;
    struct FrameStatistics
    {
        Statistics data; //!< all events for one frame
        eq::Viewport region; //!< from draw for equalizers
        /** reference count by pipe and transmit thread */
        lunchbox::a_int32_t used;
    };

    typedef std::vector< FrameStatistics > StatisticsRB;
    typedef StatisticsRB::const_iterator StatisticsRBCIter;

    /** Global statistics events, index per frame and channel. */
    lunchbox::Lockable< StatisticsRB, lunchbox::SpinLock > statistics;

    /** The initial channel size, used for view resize events. */
    Vector2i initialSize;

    /** The application-declared regions of interest, merged if
        necessary to be non overlapping. */
    PixelViewports regions;

    /** The number of the last finished frame. */
    lunchbox::Monitor< uint32_t > finishedFrame;

    /** Listeners that get notified on each new rendered image */
    typedef std::vector< ResultImageListener* > ResultImageListeners;
    ResultImageListeners resultImageListeners;

    /** Image of the current framebuffer if result listeners are present */
    eq::Image framebufferImage;

#ifdef EQUALIZER_USE_DEFLECT
    deflect::Proxy* _deflectProxy;
#endif

    /** Dumps images when the channel is configured to do so */
    FileFrameWriter frameWriter;

    bool _updateFrameBuffer;
};

}
}
