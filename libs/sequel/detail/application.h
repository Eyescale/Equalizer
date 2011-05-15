
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

#include <eq/sequel/types.h>
#include <eq/nodeFactory.h> // base class

namespace seq
{
namespace detail
{
    /** The internal implementation for the main application object. */
    class Application : public eq::NodeFactory
    {
    public:
        Application( ApplicationPtr app );
        ~Application();
        
        bool isInitialized() const { return _config != 0; }
        bool init();
        bool exit();

        bool mapData( const uint128_t& initID );
        void unmapData();

    private:
        ApplicationPtr _app;
        Config* _config;
        ObjectMap* _objects;

        virtual eq::Node* createNode( eq::Config* parent );
        virtual eq::Pipe* createPipe( eq::Node* parent );
    };
}
}

#endif // EQSEQUEL_DETAIL_APPLICATION_H
