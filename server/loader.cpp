
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "loader.h" 

#include "compound.h" 
#include "config.h" 
#include "configVisitor.h" 

using namespace std;

namespace eq
{
namespace server
{

namespace
{
class UnusedOutputChannelFinder : public ConfigVisitor
{
public:
    UnusedOutputChannelFinder() : _candidate( 0 ) {}

    virtual VisitorResult visit( Channel* channel )
        {
            if( _candidate ) // testing a candidate (see below)
                return TRAVERSE_CONTINUE;

            const View* view = channel->getView();
            if( !view )
                return TRAVERSE_CONTINUE;

            // see if it is used as a destination channel
            _candidate = channel;
             Config* config = channel->getConfig();
            config->accept( *this );

            if( _candidate ) // survived - not a destination channel yet
                _channels.push_back( _candidate );
            _candidate = 0;

            return TRAVERSE_CONTINUE;
        };

    virtual VisitorResult visit( Compound* compound )
        {
            if( !_candidate ) // not testing a candidate (see above)
                return TRAVERSE_PRUNE;
            
            const Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;

            if( _candidate == channel )
            {
                _candidate = 0; // channel already used
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_PRUNE; // only check destination channels
        }

    const ChannelVector& getResult() const { return _channels; }

private:
    Channel* _candidate;
    ChannelVector _channels;
};

}

void Loader::addOutputCompounds( ServerPtr server )
{
    const ConfigVector& configs = server->getConfigs();
    for( ConfigVector::const_iterator i = configs.begin(); 
         i != configs.end(); ++i )
    {
        UnusedOutputChannelFinder finder;
        Config* config = *i;
        config->accept( finder );

        const ChannelVector& channels = finder.getResult();
        if( channels.empty( ))
            continue;

        Compound* group = new Compound;
        config->addCompound( group );

        for( ChannelVector::const_iterator j = channels.begin(); 
             j != channels.end(); ++j )
        {
            Compound* compound = new Compound;
            group->addChild( compound );

            Channel* channel = *j;
            compound->setChannel( channel );
        }
    }
}

}
}
