
/* Copyright (c) 2011, Fatih Erol
 *               2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#include "guiDock.h"

#include "guiPackets.h"
#include "config.h"
#include "volumeInfo.h"

#include "../renderer/transferFunction.h"

#include <co/connectionDescription.h>
#include <boost/concept_check.hpp>


namespace massVolVis
{

GUINode::GUINode()
    : co::LocalNode()
    , _config( 0 )
    , _tfTmpPtr( new TransferFunction( ))
{
}


bool GUINode::init( Config* config )
{
    LBASSERT( config );
    _config = config;

    if( !co::LocalNode::listen( ))
        return false;

    registerCommand( CMD_GUI_SETX,
                    co::CommandFunc<GUINode>( this, &GUINode::_cmdSetX ),
                    getCommandThreadQueue( ));

    registerCommand( CMD_GUI_SET_FILE,
                    co::CommandFunc<GUINode>( this, &GUINode::_cmdSetFile ),
                    getCommandThreadQueue( ));

    registerCommand( CMD_GUI_SET_TF,
                    co::CommandFunc<GUINode>( this, &GUINode::_cmdSetTF ),
                    getCommandThreadQueue( ));

    return true;
}


bool GUINode::_cmdSetX( co::ICommand &command )
{
    int x;
    command >> x;

    LBWARN << "_cmdSetX: " << x << std::endl;

    return true;
}


bool GUINode::_cmdSetFile( co::ICommand &command )
{
    std::string fileName = command.get<std::string>();

    VolumeInfo& volInfo = _config->getVolumeInfo();
    volInfo.setModelFileName( fileName );
    volInfo.commit();

    return true;
}


bool GUINode::_cmdSetTF( co::ICommand &command )
{
    command >> _tfTmpPtr->rgba >> _tfTmpPtr->sda;

    VolumeInfo& volInfo = _config->getVolumeInfo();
    volInfo.setTransferFunction( *_tfTmpPtr  );
    volInfo.commit();

    return true;
}

//------------- GUINode ------------------------

bool GUIDock::init( Config* config )
{
    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_TCPIP;
    desc->setHostname( VOL_VIS_GUI_DEFAULT_HOST );
    desc->port = VOL_VIS_GUI_DEFAULT_PORT;

    _guiNode = new GUINode;
    _guiNode->addConnectionDescription( desc );
    if( !_guiNode->init( config ))
    {
        LBERROR << "Can't initialize GUIDock" << std::endl;
        return false;
    }

    LBWARN << "GUIDock listening on " << desc->getHostname() << ":" << desc->port << std::endl;
    return true;
}

GUIDock::~GUIDock()
{
    _guiNode->close();
}


}// namespace massVolVis
