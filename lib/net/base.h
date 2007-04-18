
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/net/commandFunc.h>

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/base/nonCopyable.h>
#include <eq/base/requestHandler.h>

namespace eqNet
{
    class Connection;
    class Command;
    enum  CommandResult;

    /** 
     * The base class for all networked objects. 
     *
     * Provides packet dispatch for an object using a command handler
     * table. Handles the result of the command handlers.
     */
    class EQ_EXPORT Base
    {
    public:
        Base();
		Base( const Base& from );
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
         * @param command the command.
         * @param func the functor to handle the command.
         */
        template< typename T >
        void registerCommand( const uint32_t command, 
                              const CommandFunc<T>& func);

        
        /** 
         * The default handler for handling commands.
         * 
         * @param command the command
         * @return the result of the operation.
         */
        CommandResult _cmdUnknown( Command& command );

        /**
         * The command handler which requests the command to be pushed to
         * another entity.
         */
        CommandResult _cmdPush( Command& )
            { return eqNet::COMMAND_PUSH; }

    private:
        void _registerCommand( const uint32_t command, 
                               const CommandFunc<Base>& func);

#pragma warning(push)
#pragma warning(disable: 4251)
        /** The command handler function table. */
        std::vector< CommandFunc<Base> > _vTable;
#pragma warning(pop)
    };

    template< typename T >
    void Base::registerCommand( const uint32_t command,
                                const CommandFunc<T>& func)
    {
        _registerCommand( command, static_cast< CommandFunc<Base> >( func ));
    }
}

#endif // EQNET_BASE_H
