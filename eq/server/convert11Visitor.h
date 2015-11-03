
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
class ResetSegmentEyes : public ServerVisitor
{
    virtual VisitorResult visit( Segment* segment )
    {
        if( segment->getEyes() == 0 )
            segment->setEyes( EYES_ALL );
        return TRAVERSE_CONTINUE;
    }
};

class ConvertTo11Visitor : public ServerVisitor
{
    virtual VisitorResult visitPre( Config* config )
    {
        const float version = config->getFAttribute( Config::FATTR_VERSION );
        if( version >= 1.1f )
            return TRAVERSE_PRUNE;

        ServerPtr server = config->getServer();
        if( !server->getConnectionDescriptions().empty( ))
            return TRAVERSE_CONTINUE;

        const Nodes& nodes = config->getNodes();
        if( nodes.size() > 1 )
        {
            // RFE 3156103: Add default appNode connection for multi-node config
            LBINFO << "Adding default server connection for multi-node config"
                   << std::endl;
            co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
            desc->port = 0; // Let the OS choose the port.
            server->addConnectionDescription( desc );
        }
        return TRAVERSE_CONTINUE;
    }
    virtual VisitorResult visitPost( Config* config )
    {
        // reset eyes of compound-less segments
        ResetSegmentEyes resetEyes;
        config->accept( resetEyes );

        // set new version
        config->setFAttribute( Config::FATTR_VERSION, 1.1f );
        Global::instance()->setConfigFAttribute( Config::FATTR_VERSION, 1.1f );
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visitPre( Node* node )
    {
        if( node->isApplicationNode() && 
            node->getConnectionDescriptions().empty() &&
            node->getConfig()->getNodes().size() > 1 )
        {
            //RFE 3156103: Add default appNode connection for multi-node configs
            LBINFO << "Adding default appNode connection for multi-node config"
                   << std::endl;
            node->addConnectionDescription( new ConnectionDescription );
        }
        return TRAVERSE_PRUNE;
    }

    virtual VisitorResult visit( Segment* segment )
    {
        segment->setEyes( 0 ); // eyes will be re-enabled below and above
        return TRAVERSE_CONTINUE;
    }

    virtual VisitorResult visit( Compound* compound )
    {
        if( !compound->isDestination() )
            return TRAVERSE_CONTINUE;

        Channel* channel = compound->getChannel();
        Segment* segment = channel->getSegment();
        View* view = channel->getView();

        if( segment == 0 || view == 0 ) // view-less dest compound
            return TRAVERSE_PRUNE;

        uint32_t compoundEyes = compound->getEyes();
        const uint32_t segmentEyes = segment->getEyes();
        Compound* parent = compound->getParent();

        if( compoundEyes != fabric::EYE_UNDEFINED &&
            ( compoundEyes & fabric::EYE_CYCLOP ) == 0 )
        {
            view->changeMode( View::MODE_STEREO );
            compound->enableEye( fabric::EYE_CYCLOP );
        }
        while( compoundEyes == fabric::EYE_UNDEFINED && parent )
        {
            const uint32_t parentEyes = parent->getEyes();
            if( parentEyes == fabric::EYE_UNDEFINED )
            {
                parent = parent->getParent();
                continue;
            }
            compoundEyes = parentEyes;
            if( ( parentEyes & fabric::EYE_CYCLOP ) == 0 )
            {
                view->changeMode( View::MODE_STEREO );
                parent->enableEye( fabric::EYE_CYCLOP );
            }
            parent = parent->getParent();
        }

        if( compoundEyes == fabric::EYE_UNDEFINED )
            compoundEyes = fabric::EYES_ALL;

        segment->setEyes( compoundEyes | segmentEyes | fabric::EYE_CYCLOP );
        return TRAVERSE_PRUNE;
    }
};
}
