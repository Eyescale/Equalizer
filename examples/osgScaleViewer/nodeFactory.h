
/*
 * Copyright (c)
 *   2008-2009, Thomas McGuire <thomas.mcguire@student.uni-siegen.de>
 *   2010, Stefan Eilemann <eile@equalizergraphics.com>
 *   2010, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include <eq/eq.h>

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new Channel( parent ); }

    virtual eq::Config* createConfig( eq::ServerPtr parent )
        { return new Config( parent ); }

    virtual eq::Node* createNode( eq::Config* parent )
       { return new Node( parent ); }

    virtual eq::Pipe* createPipe( eq::Node* parent )
       { return new Pipe( parent ); }
};

#endif
