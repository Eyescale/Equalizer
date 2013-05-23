
/* Copyright (c) 2011, Fatih Erol
 *               2013, David Steiner <steiner@ifi.uzh.ch>
 *
 */


#include "controller.h"

#include "ivs/src/TransferFunction.h"

#include <msv/util/hlp.h> // myClip

#include <co/connectionDescription.h>
#include <co/oCommand.h>
#include <volVis/EQ/guiPackets.h>

#include <string>


namespace massVolGUI
{


Controller::Controller( ivs::TransferFunctionPair* tf )
    : _localNode( NULL )
    , _applicationNode( NULL )
    , _x( 0 )
    , _xNew( true )
    , _tf( tf )
    , _tfNew( false )
    , _fileNameNew( false )
{
    EQASSERT( _tf );

    _localNode = new co::LocalNode;

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_TCPIP;
    desc->setHostname( "localhost" );
    _localNode->addConnectionDescription( desc );
    _localNode->listen();
}


Controller::~Controller()
{
    disconnect();
}


bool Controller::isConnected() const
{
    return (_applicationNode != NULL) && _applicationNode->isConnected();
}


bool Controller::connect( const std::string &host, short port )
{
    if( isConnected( ))
        return true;

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_TCPIP;
    desc->setHostname( host );
    desc->port = port;

    _applicationNode = new co::Node;
    _applicationNode->addConnectionDescription( desc );

    if( _localNode->connect( _applicationNode ))
    {
        _onConnect();
        return true;
    }

    return false;
}


void Controller::disconnect()
{
    if( !isConnected( ))
        return;

    _localNode->close();
}


void Controller::_onConnect()
{
    setX( _x );
}


bool Controller::setX( const int value )
{
    if( _x == value && !_xNew )
        return true;

    _x    = value;
    _xNew = true;

    if( !isConnected( ))
        return false;

    _applicationNode->send( massVolVis::CMD_GUI_SETX ) << value;
    _xNew = false;

    return true;
}


bool Controller::updateTF()
{
    _tfNew = true;

    if( !isConnected( ))
        return false;

    EQASSERT( _tf->first.size()      == 256            );
    EQASSERT( _tf->first.getFormat() == ivs::tf::RGBA8 );

    const float* data = _tf->first.getData();
    std::vector<byte> rgba( 256*4 );
    for( size_t i = 0; i < rgba.size(); ++i )
        rgba[i] = static_cast<byte>( hlpFuncs::myClip( data[i]*255.f, 0.f, 255.f ));

    data = _tf->second.getData();
    std::vector<byte> sda( 256*3 );
    for( size_t i = 0; i < sda.size(); ++i )
        sda[i] = static_cast<byte>(  hlpFuncs::myClip( data[i]*255.f, 0.f, 255.f ));

    _applicationNode->send( massVolVis::CMD_GUI_SET_TF ) << rgba << sda;
    _tfNew = false;

    return true;
}


bool Controller::loadFile( const std::string &fileName )
{
    _fileNameNew = true;

    if( !isConnected( ))
        return false;

    _applicationNode->send( massVolVis::CMD_GUI_SET_FILE ) << fileName;

    _fileNameNew = false;

    return true;
}


}// namespace massVolGUI


