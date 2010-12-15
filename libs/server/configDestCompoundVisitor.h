
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CONFIGDESTCOMPOUNDVISITOR_H
#define EQSERVER_CONFIGDESTCOMPOUNDVISITOR_H

#include "configVisitor.h" // base class
#include "compound.h" // used in inline method

namespace eq
{
namespace server
{
namespace
{

class ConfigDestCompoundVisitor : public ConfigVisitor
{
public:
    ConfigDestCompoundVisitor( const Channels& channels, const bool ao )
            : _channels( channels ), _activeOnly( ao ) {}
    ConfigDestCompoundVisitor( Channel* channel, const bool ao )
            : _activeOnly( ao ) { _channels.push_back( channel ); }
    virtual ~ConfigDestCompoundVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            // Not in our search list
            Channels::const_iterator i = stde::find( _channels, channel );
            if( i == _channels.end( ))
                return TRAVERSE_PRUNE;

            if( _activeOnly )
            {
                // Not an active compound
                const Canvas* canvas = channel->getCanvas();
                if( !canvas ||
                    canvas->getActiveLayout() != channel->getLayout( ))
                {
                    return TRAVERSE_PRUNE;
                }
            }
            _result.push_back( compound );
            return TRAVERSE_PRUNE;
        }

    const Compounds& getResult() const { return _result; }

private:
    Channels _channels;
    Compounds _result;
    const bool _activeOnly;
};

}
}
}

#endif // EQSERVER_CONFIGDESTCOMPOUNDVISITOR_H
