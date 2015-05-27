
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSEQUEL_DETAIL_APPLICATION_H
#define EQSEQUEL_DETAIL_APPLICATION_H

#include <seq/types.h>
#include <eq/nodeFactory.h> // base class

namespace seq
{
namespace detail
{
    /** The internal implementation for the main application object. */
    class Application : public eq::NodeFactory
    {
    public:
        Application( ApplicationPtr app, co::Object* initData );
        ~Application();

        Config* getConfig();
        const Config* getConfig() const;
        co::Object* getInitData() { return _initData; }

        bool isInitialized() const { return _config != 0; }
        bool isMaster() const { return _isMaster; }

        bool init();
        bool exit();
        bool run( co::Object* frameData );

    private:
        ApplicationPtr _app;
        Config* _config;
        co::Object* const _initData;
        bool _isMaster;

        virtual eq::Config* createConfig( eq::ServerPtr parent );
        virtual void releaseConfig( eq::Config* config );
        virtual eq::View* createView( eq::Layout* parent );
        virtual eq::Node* createNode( eq::Config* parent );
        virtual eq::Pipe* createPipe( eq::Node* parent );
        virtual eq::Window* createWindow( eq::Pipe* parent );
        virtual eq::Channel* createChannel( eq::Window* parent );
    };
}
}

#endif // EQSEQUEL_DETAIL_APPLICATION_H
