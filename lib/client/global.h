
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/types.h>

#include <eq/fabric/global.h>     // base class
#include <eq/fabric/iAttribute.h> // enum definition

#include <eq/base/base.h>

namespace eq
{
    class NodeFactory;

#ifdef EQ_USE_DEPRECATED
    typedef fabric::IAttribute IAttrValue;
#endif
    using fabric::UNDEFINED;
    using fabric::RGBA32F;
    using fabric::RGBA16F;
    using fabric::FBO;
    using fabric::LOCAL_SYNC;
    using fabric::DRAW_SYNC;
    using fabric::ASYNC;
    using fabric::PBUFFER;
    using fabric::WINDOW;
    using fabric::VERTICAL;
    using fabric::QUAD;
    using fabric::ANAGLYPH;
    using fabric::NICEST;
    using fabric::AUTO;
    using fabric::OFF;
    using fabric::ON;
    using fabric::FASTEST;
    using fabric::HORIZONTAL;

    /** 
     * Global parameter handling for the Equalizer client library. 
     */
    class Global : public fabric::Global
    {
    public:
        /** @return the node factory. */
        static NodeFactory* getNodeFactory() { return _nodeFactory; }

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

        static std::string  _configFile;
    };
}

#endif // EQ_GLOBAL_H

