
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PIPE_H
#define EQS_PIPE_H

#include <ostream>
#include <vector>

namespace eqs
{
    class Node;
    class Window;

    /**
     * The pipe.
     */
    class Pipe
    {
    public:
        /** 
         * Constructs a new Pipe.
         */
        Pipe();

        /** 
         * Clones this pipe.
         * 
         * @return the cloned pipe.
         */
        Pipe* clone() const;

        /** 
         * Adds a new window to this config.
         * 
         * @param window the window.
         */
        void addWindow( Window* window ){ _windows.push_back( window ); }

        /** 
         * Removes a window from this config.
         * 
         * @param window the window
         * @return <code>true</code> if the window was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeWindow( Window* window );

        /** 
         * Returns the number of windows on this config.
         * 
         * @return the number of windows on this config. 
         */
        uint nWindows() const { return _windows.size(); }

        /** 
         * Gets a window.
         * 
         * @param index the window's index. 
         * @return the window.
         */
        Window* getWindow( const uint index ) const
            { return _windows[index]; }

        /** 
         * References this pipe as being actively used.
         */
        void refUsed();

        /** 
         * Unreferences this pipe as being actively used.
         */
        void unrefUsed();

        /** 
         * Returns if this pipe is actively used.
         *
         * @return <code>true</code> if this pipe is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

    private:
        /** The list of windows. */
        std::vector<Window*>   _windows;

        /** Number of entitities actively using this pipe. */
        uint _used;

        /** The parent node. */
        Node* _node;
        friend class Node;
    };

    inline std::ostream& operator << ( std::ostream& os, const Pipe* pipe)
    {
        if( !pipe )
        {
            os << "NULL pipe";
            return os;
        }

        const uint nWindows = pipe->nWindows();
        os << "pipe " << (void*)pipe << " " << nWindows << " windows";

        for( uint i=0; i<nWindows; i++ )
            os << std::endl << "    " << pipe->getWindow(i);

        return os;
    }
};
#endif // EQS_PIPE_H
