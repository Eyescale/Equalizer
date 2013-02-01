
/* Copyright (c) 2013, Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "sageProxy.h"

#include "channel.h"
#include "pipe.h"
#include "view.h"
#include "window.h"

#include <libsage.h>
#ifdef GLX
#  include "glx/sageEventHandler.h"
#endif

namespace eq
{
namespace detail
{
class SageProxy
{
public:
    SageProxy( eq::Channel* channel )
        : _sail( 0 )
#ifdef GLX
        , _eventHandler( 0 )
#endif
        , _channel( channel )
        , _isRunning( false )
    {
        const std::string& configFile = channel->getView()->getSageConfig();
        sage::initUtil();
        sailConfig scfg;
        if( scfg.init( const_cast< char* >( configFile.c_str( ))) == -1 )
            return;

        std::string appName = channel->getView()->getName();
        if( appName.empty( ))
            appName = configFile;
        scfg.setAppName( const_cast< char* >( appName.c_str( )));
        scfg.rowOrd = BOTTOM_TO_TOP;
        scfg.resX = scfg.winWidth = channel->getPixelViewport().w;
        scfg.resY = scfg.winHeight = channel->getPixelViewport().h;

        _sail = new sail;
        // TODO: init blocks if fsManager is not up; maybe async invoke?
        if( _sail->init( scfg ) == -1 )
            delete _sail;
        _isRunning = true;
    }

    ~SageProxy()
    {
        if( !_sail )
            return;

    #ifdef GLX
        delete _eventHandler;
    #endif

        _sail->shutdown();
        delete _sail;
    }

    void swapBuffer()
    {
        if( !_sail )
            return;

        const PixelViewport& pvp = _channel->getWindow()->getPixelViewport();
        unsigned char* buffer = nextBuffer( _sail );
        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        glReadPixels( 0, 0, pvp.w, pvp.h, GL_RGB, GL_UNSIGNED_BYTE, buffer );
        ::swapBuffer( _sail );
    }

    sail* _sail;
#ifdef GLX
    glx::SageEventHandler* _eventHandler;
#endif
    eq::Channel* _channel;
    bool _isRunning;
};
}

SageProxy::SageProxy( Channel* channel )
    : _impl( new detail::SageProxy( channel ))
{
#ifdef GLX
    if( channel->getPipe()->getWindowSystem().getName() == "GLX" )
        _impl->_eventHandler = new glx::SageEventHandler( this );
#endif
}

SageProxy::~SageProxy()
{
    delete _impl;
}

void SageProxy::swapBuffer()
{
    _impl->swapBuffer();
}

Channel* SageProxy::getChannel()
{
    return _impl->_channel;
}

sail* SageProxy::getSail()
{
    return _impl->_sail;
}

void SageProxy::stopRunning()
{
    _impl->_isRunning = false;
}

bool SageProxy::isRunning() const
{
    return _impl->_isRunning;
}

}
