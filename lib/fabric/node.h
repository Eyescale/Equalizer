
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 *               2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_NODE_H
#define EQFABRIC_NODE_H

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/object.h>        // base class
#include <eq/fabric/paths.h>
#include <eq/fabric/pixelViewport.h> // base class
#include <eq/fabric/types.h>

namespace eq
{

namespace fabric
{
    template< class, class, class > class Pipe;
    template< class C, class N, class P > class Node : public Object
    {
    public:
        typedef std::vector< P* > PipeVector;

        /** @name Data Access */
        //@{
        const PipeVector& getPipes() const { return _pipes; }

        /** @return the index path to this node. @internal */
        EQFABRIC_EXPORT NodePath getPath() const;

        /** @return the number of the last finished frame. */
        uint32_t getFinishedFrame() const { return _finishedFrame; }
        //@}

        /**
         * @name Attributes
         */
        //@{
        // Note: also update string array initialization in node.cpp
        /** Node attributes. */
        enum IAttribute
        {
            /** <a href="http://www.equalizergraphics.com/documents/design/threads.html#sync">Threading model</a> */
            IATTR_THREAD_MODEL,
            IATTR_LAUNCH_TIMEOUT,         //!< Launch timeout
            IATTR_FILL1,
            IATTR_FILL2,
            IATTR_ALL
        };

        EQFABRIC_EXPORT void setIAttribute( const IAttribute attr,
                                            const int32_t value );
        EQFABRIC_EXPORT int32_t getIAttribute( const IAttribute attr ) const;
        EQFABRIC_EXPORT static const std::string& getIAttributeString(
                                                        const IAttribute attr );
        //@}

        /** Add additional tasks this node might potentially execute. */
        void addTasks( const uint32_t tasks ) { _tasks |= tasks; }

    protected:
        Node();

        Node( const Node& from, C* config );

        EQFABRIC_EXPORT virtual ~Node(){}

        /** Pipe children. */
        PipeVector _pipes;
        
        /** The reason for the last error. */
        std::string _error;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];

        /** The number of the last finished frame. */
        uint32_t _finishedFrame;

        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class, class > friend class Pipe;
        void _addPipe( P* pipe );
        bool _removePipe( P* pipe );
        P* _findPipe( const uint32_t id );

    };
}
}

#endif // EQFABRIC_NODE_H

