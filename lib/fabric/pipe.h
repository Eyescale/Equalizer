
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQFABRIC_PIPE_H
#define EQFABRIC_PIPE_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/paths.h>
#include <eq/fabric/pixelViewport.h> // base class
#include <eq/fabric/types.h>

namespace eq
{

namespace fabric
{
    template< class N, class P, class W > class Pipe : public Object
    {
    public:
        
        /** A vector of pointers to Window */
        typedef std::vector< W* >  WindowVector; 
        
        N*       getNode()       { return _node; }
        const N* getNode() const { return _node; }

        /** @name Error Information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within configInit().
         *
         * @param message the error message.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setErrorMessage( const std::string& message );
        //@}
        
        /**
         * @name Data Access
         */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        
        /** 
         * Return the set of tasks this pipe's channels might execute in the
         * worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @warning Not finalized, might change in the future.
         * @return the tasks.
         */
        uint32_t getTasks() const { return _tasks; }

         /**
         * Returns the port number of this pipe.
         * 
         * The port number identifies the X server for systems using the
         * X11/GLX window system. It currently has no meaning on other systems.
         *
         * @return the port number of this pipe, or
         *         <code>EQ_UNDEFINED_UINT32</code>.
         */
        uint32_t getPort() const { return _port; }
        
        void setPort( const uint32_t port )      { _port = port; }
        /** 
         * Returns the device number of this pipe.
         * 
         * The device number identifies the X screen for systems using the
         * X11/GLX window system, or the number of the virtual screen for the
         * AGL window system. On Windows systems it identifies the graphics
         * adapter. Normally the device identifies a GPU.
         *
         * @return the device number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT32</code>.
         */
        uint32_t getDevice() const { return _device; }

        void setDevice( const uint32_t device )  { _device = device; }

        /** @return the pixel viewport. */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** @return the index path to this pipe. @internal */
        EQFABRIC_EXPORT PipePath getPath() const;
        //@}

        /** @return the vector of windows. */
        const WindowVector& getWindows() const { return _windows; }
        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in pipe.cpp
        enum IAttribute
        {
            IATTR_HINT_THREAD,
            IATTR_HINT_CUDA_GL_INTEROP,
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };
        
        /** @return the name of a window attribute. */
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                      const IAttribute attr );
        //@}
    protected: 

        //-------------------- Members --------------------

        /** Constructs a new pipe. */
        Pipe( N* parent );       
        
        /** Constructs a new deep copy of a pipe. */
        Pipe( const P& from, N* node );

        void _setTasks( uint32_t tasks ) { _tasks = tasks; }

        /** @return the vector of windows. */
        WindowVector& _getWindows() { return _windows; }

        void _addWindow( W* window );
        bool _removeWindow( W* window );
        W* _findWindow( const uint32_t id );

        void _setNode( N* node ){ _node = node; }

        /** The size (and location) of the pipe. */
        PixelViewport _pvp;

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

    private:
        /** The reason for the last error. */
        std::string _error;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The display (GLX) or ignored (Win32, AGL). */
        uint32_t _port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        uint32_t _device;

        /** The parent node. */
        N* _node;
        
        /** The list of windows. */
        WindowVector _windows;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQFABRIC_PIPE_H

