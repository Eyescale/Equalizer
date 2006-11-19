
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/base/base.h>
#include <eq/base/nonCopyable.h>
#include <eq/base/requestHandler.h>
#include <eq/net/commandResult.h>
#include <eq/net/packetFunc.h>

namespace eqNet
{
    class Connection;
    class Node;
    class Command;

    /** 
     * The base class for all networked objects. 
     *
     * Provides packet dispatch for an object using a command handler
     * table. Handles the result of the command handlers.
     */
    class Base : public virtual eqBase::NonCopyable
    {
    public:
        Base( const bool threadSafe = false );
        virtual ~Base();        

        /** 
         * Handles a received command packet for this object by calling the
         * appropriate command handler function.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa registerCommand
         */
        CommandResult invokeCommand( Command& command );
 
    protected:
        /** 
         * Registers a command member function for a command.
         * 
         * Use 'reinterpret_cast<CommandFcn>(foo)' to cast the function of the
         * derived class to the expected type.
         *
         * @param command the command.
         * @param func the functor to handle the command.
         */
        template< typename T >
        void registerCommand( const uint32_t command, 
                              const PacketFunc<T>& func);

        
        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** 
         * The default handler for handling commands.
         * 
         * @param node the originating node.
         * @param packet the packet.
         * @return the result of the operation.
         */
        CommandResult _cmdUnknown( Command& packet );

        /**
         * The command handler which requests the command to be pushed to
         * another entity.
         */
        CommandResult _cmdPush( Command& packet )
            { return eqNet::COMMAND_PUSH; }

        /**
         * The command handler which requests the command to be pushed to
         * another entity with high priority.
         */
        CommandResult _cmdPushFront( Command& packet )
            { return eqNet::COMMAND_PUSH_FRONT; }

    private:
        /** The command handler function table. */
        std::vector< PacketFunc<Base> > _vTable;
    };

    template< typename T >
    void Base::registerCommand( const uint32_t command,
                                const PacketFunc<T>& func)
    {
        if( command >= _vTable.size( ))
            _vTable.resize( command+1, 
                            PacketFunc<Base>( this, &Base::_cmdUnknown ));
        
        _vTable[command] = static_cast< PacketFunc<Base> >(func);
    }

}

#endif // EQNET_BASE_H
