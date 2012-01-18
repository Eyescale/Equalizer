
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
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

#ifndef EQUTIL_PIXELBUFFEROBJECT_H
#define EQUTIL_PIXELBUFFEROBJECT_H

#include <co/base/lock.h>

#include <eq/client/gl.h>   // for GLEW
#include <co/base/error.h>


namespace eq
{
namespace util
{
    /** A C++ class to abstract OpenGL pixel buffer objects.
      *
      * If multiple PBOs of the same read/write type are used in the same 
      * glContext they should be binded/mapped and unbinded/unmapped 
      * sequentially.
      *
      * If thread-safe mode is used then when buffer is binded/mapped it is
      * locked until it is unbinded/unmapped (also locked on init and destroy).
      *
      * On correct PBO usage see: http://www.songho.ca/opengl/gl_pbo.html
      */
    class PixelBufferObject 
    {
    public:
        /**
         * Construct a new Pixel Buffer Object
         *
         * @param glewContext OpenGL context
         * @param shared      if true PBO uses locks for synced access
         */
        PixelBufferObject( const GLEWContext* glewContext,
                           const bool         threadSafe );

        /** Destruct the Frame Buffer Object */
        virtual ~PixelBufferObject();

        /**
         * Initialize the Pixel Buffer Object.
         *
         * @param newSize total number of bytes (has to be > 0)
         * @param read    if true PBO is used for reading, otherwise for writing
         * @return        true on success, false otherwise
         */
        virtual bool init( const int32_t newSize,
                            const GLEWContext* glewContext,
                            const bool read = true );

        virtual bool bind(    const GLEWContext* glewContext ) const;
        virtual void unbind(  const GLEWContext* glewContext ) const;
        virtual void destroy( const GLEWContext* glewContext );

        /**
         * Binds the PBO and mappes its data for reading.
         * Supposed to be used only when reading from PBO.
         *
         * @return pointer to the PBO memory
         */
        virtual const void* mapRead( const GLEWContext* glewContext ) const;

        /**
         * Binds the PBO and mappes its data for writing.
         * Supposed to be used only when writing to PBO
         *
         * @return pointer to the PBO memory
         */
        virtual void* mapWrite( const GLEWContext* glewContext );

        /**
         * Unmaps PBO and unbinds it.
         *
         * @return pointer to the PBO memory
         */
        virtual void unmap( const GLEWContext* glewContext ) const;

        int32_t getDataSize() const { return _size; }


        /** @return the reason for the last failed operation. */
        const co::base::Error& getError() const { return _error; }

        /** @return true if the pbo is intialized. */
        bool isInitialized() const { return _initialized; }

        /** @return true if the access to pbo is blocking */
        bool isThreadSafe() const { return _threadSafe; }

    private:

        /**
         * Sets error if PBO is not initialized
         *
         * @return true if initialized, false otherwise
         */
        bool _testIfInitialized() const;

        bool _init( const int32_t newSize, const GLEWContext* glewContext,
                                                            const bool read );

        bool _bind(     const GLEWContext* glewContext ) const;
        void _unbind(   const GLEWContext* glewContext ) const;
        void _unmap(    const GLEWContext* glewContext ) const;
        void _destroy(  const GLEWContext* glewContext );


        inline void _lock()   const { if( _threadSafe ) _pboLock.set();   }
        inline void _unlock() const { if( _threadSafe ) _pboLock.unset(); }

        GLuint  _pboId; //!< the PBO GL name
        int32_t _size;  //!< size of the allocated PBO buffer

        GLuint _name;//!< GL_PIXEL_PACK_BUFFER_ARB or GL_PIXEL_UNPACK_BUFFER_ARB
        GLuint _type;//!< GL_READ_ONLY_ARB         or GL_WRITE_ONLY_ARB
        GLuint _op;  //!< GL_STREAM_READ_ARB       or GL_STREAM_DRAW_ARB

        bool   _initialized; // true if PBO is fully initialized
        bool   _threadSafe;  // if "true" PBO uses locks to share access

        const GLEWContext* _initCtx;//used to delete PBO if object is destroyed

        mutable co::base::Error _error;   //!< The reason for the last error
        mutable co::base::Lock  _pboLock;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        void _setError( const int32_t error ) const;
    };
}
}


#endif // EQUTIL_PIXELBUFFEROBJECT_H 
