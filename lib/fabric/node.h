
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

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/types.h>
#include <eq/fabric/visitorResult.h> // enum

namespace eq
{
    class Node;
    namespace server { class Node; }

namespace fabric
{
    struct NodePath;

    template< class C, class N, class P, class V > class Node : public Object
    {
    public:
        typedef std::vector< P* > PipeVector;

        /** @name Data Access */
        //@{
        /** @return vector of pipes */
        const PipeVector& getPipes() const { return _pipes; }

        /** @return the config of this node. */
        C*       getConfig()       { return _config; }
        const C* getConfig() const { return _config; }

        /**
         * @return true if all render tasks for this node are executed by the
         *         application process, false otherwise.
         * @internal
         */
        bool isApplicationNode() const { return _isAppNode; }
        void setApplicationNode( const bool isAppNode ); //!< @internal

        /** @return the index path to this node. @internal */
        EQFABRIC_EXPORT NodePath getPath() const;
        P* findPipe( const uint32_t id ); //!< @internal

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor );

        /** Const-version of accept(). */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor ) const;
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

        EQFABRIC_EXPORT virtual void backup(); //!< @internal
        EQFABRIC_EXPORT virtual void restore(); //!< @internal

    protected:
        Node( C* parent );
        EQFABRIC_EXPORT virtual ~Node();

        virtual ChangeType getChangeType() const { return UNBUFFERED; }
        
        /** @internal */
        EQFABRIC_EXPORT virtual void serialize( net::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_EXPORT virtual void deserialize( net::DataIStream& is, 
                                                  const uint64_t dirtyBits );

    private:
        enum DirtyBits
        {
            DIRTY_ATTRIBUTES      = Object::DIRTY_CUSTOM << 0,
            DIRTY_MEMBER          = Object::DIRTY_CUSTOM << 1,
        };

        /** Pipe children. */
        PipeVector _pipes;

        /** The parent config. */
        C* const _config;

        struct BackupData
        {
            /** Integer attributes. */
            int32_t iAttributes[IATTR_ALL];
        }
            _data, _backup;

        bool _isAppNode; //!< execute render tasks in application process

        //friend class eq::Node; // TODO remove
        //friend class eq::server::Node; // TODO remove

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class, class, class > friend class Pipe;
        void _addPipe( P* pipe );
        bool _removePipe( P* pipe );
    };
}
}

#endif // EQFABRIC_NODE_H

