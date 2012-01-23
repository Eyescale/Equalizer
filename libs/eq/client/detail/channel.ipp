
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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
            , fbo( 0 )
            , initialSize( Vector2i::ZERO )
        {
            co::base::RNG rng;
            color.r() = rng.get< uint8_t >();
            color.g() = rng.get< uint8_t >();
            color.b() = rng.get< uint8_t >();
        }

    ~Channel()
        {
            statistics->clear();
            EQASSERT( !fbo );
        }

    /** The channel's drawable config (FBO). */
    DrawableConfig drawableConfig;

    /** The configInit/configExit state. */
    State state;

    /** server-supplied vector of output frames for current task. */
    Frames outputFrames;

    /** Server-supplied vector of input frames for current task. */
    Frames inputFrames;

    /** Used as an alternate drawable. */
    util::FrameBufferObject* fbo; 

    /** A random, unique color for this channel. */
    Vector3ub color;

    typedef std::vector< Statistic > Statistics;
    struct FrameStatistics
    {
        Statistics data; //!< all events for one frame
        /** reference count by pipe and transmit thread */
        co::base::a_int32_t used;
    };

    typedef std::vector< FrameStatistics > StatisticsRB;
    typedef StatisticsRB::const_iterator StatisticsRBCIter;

    /** Global statistics events, index per frame and channel. */
    co::base::Lockable< StatisticsRB, co::base::SpinLock > statistics;

    /** The initial channel size, used for view resize events. */
    Vector2i initialSize;

    /** The application-declared region of interest. */
    eq::PixelViewport region;
};

}
}
