
/* Copyright (c) 2013-2016, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// dumps the thread affinity setting of all locally-found GPUs.

#include <eq/eq.h>

namespace eq
{
namespace detail
{
class ThreadAffinityVisitor : public eq::ConfigVisitor
{
public:
    ThreadAffinityVisitor() {}
    virtual ~ThreadAffinityVisitor() {}

    virtual VisitorResult visitPre( eq::Pipe* pipe )
    {
        std::cout << "GPU " << pipe->getPort() << "." << pipe->getDevice()
                  << ": "
                  << lunchbox::Thread::Affinity( pipe->_getAutoAffinity( ))
                  << std::endl;
        return TRAVERSE_PRUNE;
    }
};
}
}

int main( int argc, char **argv )
{
    int retval = EXIT_FAILURE;

    eq::NodeFactory nodeFactory;
    if( eq::init( argc, argv, &nodeFactory ))
    {
        eq::ClientPtr client = new eq::Client;
        if( client->initLocal( argc, argv ))
        {
            eq::ServerPtr server = new eq::Server;
            if( client->connectServer( server ))
            {
                eq::fabric::ConfigParams configParams;
                eq::Global::setConfig( "local" );
                eq::Config* config = server->chooseConfig( configParams );
                if( config )
                {
                    if( config->init( co::uint128_t( )))
                    {
                        eq::detail::ThreadAffinityVisitor visitor;
                        config->accept( visitor );
                        retval = EXIT_SUCCESS;
                        config->exit();
                    }
                    server->releaseConfig( config );
                }
                else
                    LBERROR << "No matching config on server" << std::endl;

                client->disconnectServer( server );
            }
            else
                LBERROR << "Can't open server" << std::endl;

            client->exitLocal();
            LBASSERTINFO( client->getRefCount() == 1, client );
        }
        eq::exit();
    }
    else
        LBERROR << "Equalizer init failed" << std::endl;

    return retval;
}
