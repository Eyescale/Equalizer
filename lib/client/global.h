
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <eq/base/base.h>
#include <eq/base/lock.h> // member
#include <eq/client/types.h>
#include <string>

namespace eq
{
    class NodeFactory;
    
    /** Possible values for integer attributes */
    enum IAttrValue
    {
        UNDEFINED  = -0xfffffff, //!< Undefined value
        RGBA32F    = -13, //!< Float32 framebuffer (Window::IATTR_PLANES_COLOR)
        RGBA16F    = -12, //!< Float16 framebuffer (Window::IATTR_PLANES_COLOR)
        FBO        = -11, //!< FBO drawable (Window::IATTR_HINT_DRAWABLE)
        LOCAL_SYNC = -10, //!< Full local sync (Node::IATTR_THREAD_MODEL)
        DRAW_SYNC  = -9,  //!< Local draw sync (Node::IATTR_THREAD_MODEL)
        ASYNC      = -8,  //!< No local sync (Node::IATTR_THREAD_MODEL)
        PBUFFER    = -7,  //!< PBuffer drawable (Window::IATTR_HINT_DRAWABLE)
        WINDOW     = -6,  //!< Window drawable (Window::IATTR_HINT_DRAWABLE)
        VERTICAL   = -5,  //!< Vertical load-balancing
        QUAD       = -4,  //!< Quad-buffered stereo decomposition
        ANAGLYPH   = -3,  //!< Anaglyphic stereo decomposition
        /** 
         * Nicest statisics gathering (Window::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        NICEST     = -2,
        AUTO       = -1,  //!< Automatic selection (various attributes)
        OFF        = 0,   //!< disabled (various attributes)
        ON         = 1,   //!< enabled (various attributes)
        /** 
         * Fastest statisics gathering (Window::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        FASTEST    = ON,
        HORIZONTAL = ON   //!< Horizontal load-balancing
    };

    /** 
     * Global parameter handling for the Equalizer client library. 
     */
    class Global
    {
    public:
        /** @return the node factory. */
        static NodeFactory* getNodeFactory() { return _nodeFactory; }

        /** 
         * Set the default Equalizer server.
         * 
         * @param server the default server.
         */
        EQ_EXPORT static void setServer( const std::string& server );

        /** @return the default Equalizer server. */
        EQ_EXPORT static const std::string& getServer();

        /** 
         * Set the config file for the app-local server.
         * 
         * @param configFile the default configFile.
         */
        EQ_EXPORT static void setConfigFile( const std::string& configFile );

        /** @return the default config file for the app-local server. */
        EQ_EXPORT static const std::string& getConfigFile();

        /** 
         * Global lock for all non-thread-safe Carbon API calls. 
         * Note: this is a nop on non-AGL builds. Do not use unless you know the
         * side effects, i.e., ask on the eq-dev mailing list.
         */
        static void enterCarbon();
        /** Global unlock for all non-thread-safe Carbon API calls */
        static void leaveCarbon();
        
    private:
        EQ_EXPORT friend bool init( const int argc, char** argv, 
                                    NodeFactory* nodeFactory );
        EQ_EXPORT friend bool exit();
        
        static NodeFactory* _nodeFactory;

        static std::string  _server;
        static std::string  _configFile;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const IAttrValue value );
}

#endif // EQ_GLOBAL_H

