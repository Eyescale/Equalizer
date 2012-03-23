
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

#include <test.h>
#include <seq/sequel.h>
#include <stdlib.h>

#ifdef _WIN32
#  define setenv( name, value, overwrite ) \
    SetEnvironmentVariable( name, value )
#endif

class Renderer : public seq::Renderer
{
public:
    Renderer( seq::Application& application ) : seq::Renderer( application ) {}
    virtual ~Renderer() {}

protected:
    virtual void draw( co::Object* frameData )
        {
            lunchbox::sleep( 20000 );
            getApplication().stopRunning();
        }
};

class Application : public seq::Application
{
public:
    virtual ~Application() {}
    virtual seq::Renderer* createRenderer() { return new Renderer( *this ); }
    virtual co::Object * createObject( const uint32_t type )
        { EQUNIMPLEMENTED; return 0; }
};
typedef lunchbox::RefPtr< Application > ApplicationPtr;

int main( const int argc, char** argv )
{
    setenv( "EQ_CONFIG_IATTR_ROBUSTNESS", "0", 1 /* overwrite */ );
    setenv( "CO_TIMEOUT", "1000", 1 /* overwrite */ );
    ApplicationPtr app = new Application;
    lunchbox::Clock clock;

    if( app->init( argc, argv, 0 ) && app->run( 0 ) && app->exit( ))
        return EXIT_SUCCESS;

    TEST( clock.getTime64() > 20000 );
    TEST( clock.getTime64() > 22000 );
    return EXIT_FAILURE;
}
