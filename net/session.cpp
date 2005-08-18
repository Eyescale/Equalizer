/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"
#include "connection.h"
#include "connectionDescription.h"
#include "sessionPriv.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqNet;
using namespace std;

Session::Session(const uint id, Node* node )
        : eqNet::Session(id),
          _userID(1),
          _node(node),
          _localNode(NULL)
{
    for( int i=0; i<CMD_SESSION_ALL; i++ )
        _cmdHandler[i] = &eqNet::priv::Session::_cmdUnknown;

    _cmdHandler[CMD_SESSION_NEW_USER] = &eqNet::priv::Session::_cmdNewUser;

    INFO << "New session" << this << endl;
}

void Session::handlePacket( Node* node, const SessionPacket* packet)
{
    INFO << "handle " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_SESSION:
            ASSERT( packet->command < CMD_SESSION_ALL );
            (this->*_cmdHandler[packet->command])( connection, node, packet );
            break;

        case DATATYPE_USER:
        {
            UserPacket* userPacket = (UserPacket*)(packet);
            User* user = _users[userPacket->userID];
            ASSERT( user );

            //user->handlePacket( connection, node, userPacket );
        } break;

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Session::_cmdNewUser( Connection* connection, Node* node, Packet* pkg )
{
    SessionNewUserPacket* packet  = (SessionNewUserPacket*)pkg;
    INFO << "Cmd new user: " << packet << endl;
    
   if( packet->userID == INVALID_ID )
        packet->userID = _userID++;

    User* user = new User( packet->userID );
    _users[packet->userID] = user;
    packet->result = packet->userID;
}

void Session::pack( const NodeList& nodes )
{
    ASSERT( _node );

    for( IDHash<User*>::iterator iter = _users.begin(); iter != _users.end();
         iter++ )
    {
        SessionNewUserPacket newUserPacket;
        newUserPacket.sessionID = getID();
        newUserPacket.userID    = (*iter).first;
        nodes.send( newUserPacket );
    };
}


std::ostream& operator << ( std::ostream& os, Session* session )
{
    if( !session )
    {
        os << "NULL session";
            return os;
        }
    
    os << "    session " << session->getID() << "(" << (void*)session
       << "): " << session->_users.size() << " user[s], " 
       << " local " << session->_localNode;
    
    for( IDHash<User*>::iterator iter = session->_users.begin();
         iter != session->_users.end(); iter++ )
    {
                User* user = (*iter).second;
                os << std::endl << "    " << user;
    }
    
    return os;
}
