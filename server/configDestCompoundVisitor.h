
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
    ConfigDestCompoundVisitor( const Channels& channels, Compounds& result )
            : _channels( channels ), _compounds( result ) {}
    virtual ~ConfigDestCompoundVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            Channels::const_iterator i = stde::find( _channels, channel );
            if( i != _channels.end( ))
                _compounds.push_back( compound );
            return TRAVERSE_PRUNE;
        }

private:
    const Channels& _channels;
    Compounds& _compounds;
};

}
}
}

#endif // EQSERVER_CONFIGDESTCOMPOUNDVISITOR_H
