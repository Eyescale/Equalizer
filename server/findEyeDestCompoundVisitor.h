
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQSERVER_FINDEYEDESTCOMPOUNDVISITOR_H
#define EQSERVER_FINDEYEDESTCOMPOUNDVISITOR_H

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
    ConfigDestCompoundVisitor( const Channel* channel, Compounds& result )
            : _channel( channel )
            , _layout( channel->getLayout( ))
            , _canvas( channel->getCanvas( ))
            , _compounds( result )
    {
        EQASSERT( _canvas );
    }

    virtual ~ConfigDestCompoundVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            if( _channel != channel )
                return TRAVERSE_PRUNE;
          
            EQASSERT( compound->isDestination( ));

            if( !_canvas || _canvas->getActiveLayout() != _layout )
                return TRAVERSE_PRUNE;

            _compounds.push_back( compound );
            return TRAVERSE_PRUNE;
        }

private:
    const Channel* _channel;
    const Layout* _layout;
    const Canvas* _canvas;
    Compounds& _compounds;
};

}
}
}

#endif // EQSERVER_FINDEYEDESTCOMPOUND_H
