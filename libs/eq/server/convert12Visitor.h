
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

namespace
{
class ConvertTo12Visitor : public ServerVisitor
{
    virtual VisitorResult visitPre( Config* config )
    {
        const float version = config->getFAttribute( Config::FATTR_VERSION );
        if( version >= 1.2f )
            return TRAVERSE_PRUNE;

        // set new version
        config->setFAttribute( Config::FATTR_VERSION, 1.2f );
        Global::instance()->setConfigFAttribute( Config::FATTR_VERSION, 1.2f );
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visitPre( Node* node )
    {
        if( !node->getHost().empty( ))
        {
            LBWARN << "Pre-1.2 configurations should not have a node host name "
                   << "set: " << node->getHost() << std::endl;
            return TRAVERSE_PRUNE;
        }

        const co::ConnectionDescriptions& descs = 
            node->getConnectionDescriptions();
        for( co::ConnectionDescriptionsCIter i = descs.begin();
             i != descs.end(); ++i )
        {
            const std::string& hostname = (*i)->getHostname();
            if( hostname.empty( ))
                continue;

            node->setHost( hostname );
            return TRAVERSE_PRUNE;
        }
        return TRAVERSE_PRUNE;
    }
};
}
