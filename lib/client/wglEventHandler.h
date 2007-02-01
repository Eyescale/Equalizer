/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLEVENTHANDLER_H
#define EQ_WGLEVENTHANDLER_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>
#include <eq/client/windowEvent.h>

namespace eq
{
    /**
     * The event processing for wgl pipes.
     *
     * The WGL implementation does not use a thread, since messages are handled
     * by a 'wndproc' callback in the thread which created the window.
     */
    class EQ_EXPORT WGLEventHandler : public EventHandler
    {
    public:
        /** Constructs a new wgl event thread. */
        WGLEventHandler();

        /** Destructs the wgl event thread. */
        virtual ~WGLEventHandler(){}
        
        /** @sa EventHandler::addPipe. */
        virtual void addPipe( Pipe* pipe );
        /** @sa EventHandler::removePipe. */
        virtual void removePipe( Pipe* pipe );

        /** @sa EventHandler::addWindow. */
        virtual void addWindow( Window* window );
        /** @sa EventHandler::removeWindow. */
        virtual void removeWindow( Window* window );

        static LONG WINAPI wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                    LPARAM lParam );
    private:
#pragma warning(push)
#pragma warning(disable: 4251)
        std::vector<Pipe*> _pipes;
        eqBase::PtrHash< const Window*, uint32_t > _buttonStates;
#pragma warning(pop)

        LONG WINAPI _wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                              LPARAM lParam );

        Window*   _findWindow( HWND hWnd );
        void      _syncButtonState( const Window* window, WPARAM wParam );
        uint32_t  _getKey( LPARAM lParam, WPARAM wParam );
    };
}

#endif // EQ_WGLEVENTHANDLER_H

