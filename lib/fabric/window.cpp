
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/fabric/task.h>
#include <eq/fabric/window.h>
#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

namespace eq
{
namespace fabric
{

namespace
{
#define MAKE_WINDOW_ATTR_STRING( attr ) ( std::string("EQ_WINDOW_") + #attr )
std::string _iWindowAttributeStrings[] = {
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SWAPSYNC ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_DRAWABLE ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_WINDOW_ATTR_STRING( IATTR_HINT_SCREENSAVER ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_STENCIL ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_ACCUM_ALPHA ),
    MAKE_WINDOW_ATTR_STRING( IATTR_PLANES_SAMPLES ),
    MAKE_WINDOW_ATTR_STRING( IATTR_FILL1 ),
    MAKE_WINDOW_ATTR_STRING( IATTR_FILL2 )
};
}

template< typename P, typename W, typename C >
Window< P, W, C >::Window( P* parent )
    : _tasks( fabric::TASK_NONE ) 
    , _pipe( parent )
{
    EQASSERT( parent );
}

template< typename P, typename W, typename C >
Window< P, W, C >::Window( const W& from, P* parent ) 
    : Object()
    , _tasks( fabric::TASK_NONE )
    , _pipe( parent )
{
    EQASSERT( parent );
}

template< typename P, typename W, typename C >
void Window< P, W, C >::setErrorMessage( const std::string& message )
{
    if( _error == message )
        return;
    _error = message;
}

template< typename P, typename W, typename C >
void Window< P, W, C >::_addChannel( C* channel )
{
    EQASSERT( channel->getWindow() == this );
    _channels.push_back( channel );
}

template< typename P, typename W, typename C >
bool Window< P, W, C >::_removeChannel( C* channel )
{
    ChannelVector& channels = _getChannels();
    typename ChannelVector::iterator iter = find( channels.begin(), 
                                                  channels.end(),
                                                  channel );
    if( iter == channels.end( ))
        return false;

    channels.erase( iter );
    return true;
}

template< typename P, typename W, typename C >
C* Window< P, W, C >::_findChannel( const uint32_t id )
{
    for( typename ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        C* channel = *i;
        if( channel->getID() == id )
            return channel;
    }
    return 0;
}

template< typename P, typename W, typename C >
const std::string&  Window< P, W, C >::getIAttributeString( const IAttribute attr )
{
    return _iWindowAttributeStrings[attr];
}

}
}
