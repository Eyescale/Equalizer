
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
namespace fabric
{
    /** Base data transport class for nodes. @sa eq::Node */
    template< class C, class N, class P, class V > class Node : public Object
    {
    public:
        /** A vector of pointers to pipes. @version 1.0 */
        typedef std::vector< P* > Pipes;

        /** @name Data Access */
        //@{
        /** @return the config of this node. @version 1.0 */
        C*       getConfig()       { return _config; }

        /** @return the config of this node. @version 1.0 */
        const C* getConfig() const { return _config; }

        /** @return the vector of child pipes. @version 1.0 */
        const Pipes& getPipes() const { return _pipes; }

        /**
         * @internal
         * @return true if all render tasks for this node are executed by the
         *         application process, false otherwise.
         */
        bool isApplicationNode() const { return _isAppNode; }

        /** @internal */
        EQFABRIC_INL void setApplicationNode( const bool isAppNode );

        /** @internal @return the index path to this node. */
        EQFABRIC_INL NodePath getPath() const;
        P* findPipe( const co::base::UUID& id ); //!< @internal

        /** 
         * Traverse this node and all children using a node visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_INL VisitorResult accept( V& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_INL VisitorResult accept( V& visitor ) const;
        //@}

        /** @name Attributes */
        //@{
        // Note: also update string array initialization in node.ipp
        /** Integer attributes. */
        enum IAttribute
        {
            /** <a href="http://www.equalizergraphics.com/documents/design/threads.html#sync">Threading model</a> */
            IATTR_THREAD_MODEL,
            IATTR_LAUNCH_TIMEOUT, //!< Timeout when auto-launching the node
            IATTR_LAST,
            IATTR_ALL = IATTR_LAST + 5
        };

        /** @internal Set a node integer attribute. */
        EQFABRIC_INL void setIAttribute( const IAttribute attr,
                                            const int32_t value );

        /** @return the value of a node integer attribute. @version 1.0 */
        EQFABRIC_INL int32_t getIAttribute( const IAttribute attr ) const;

        /** @internal @return the name of a node integer attribute. */
        static const std::string& getIAttributeString( const IAttribute attr );
        //@}

        EQFABRIC_INL virtual void backup(); //!< @internal
        EQFABRIC_INL virtual void restore(); //!< @internal
        void create( P** pipe ); //!< @internal
        void release( P* pipe ); //!< @internal
        virtual void output( std::ostream& ) const {} //!< @internal

    protected:
        /** @internal Construct a new node. */
        Node( C* parent );
        /** @internal Destruct the node. */
        EQFABRIC_INL virtual ~Node();

        /** @internal */
        EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                                const uint64_t dirtyBits );
        /** @internal */
        EQFABRIC_INL virtual void deserialize( co::DataIStream& is, 
                                                  const uint64_t dirtyBits );
        
        EQFABRIC_INL virtual void notifyDetach(); //!< @internal

        /** @sa Serializable::setDirty() @internal */
        EQFABRIC_INL virtual void setDirty( const uint64_t bits );

        /** @internal */
        virtual ChangeType getChangeType() const { return UNBUFFERED; }

        enum DirtyBits
        {
            DIRTY_ATTRIBUTES      = Object::DIRTY_CUSTOM << 0,
            DIRTY_PIPES           = Object::DIRTY_CUSTOM << 1,
            DIRTY_MEMBER          = Object::DIRTY_CUSTOM << 2,
            DIRTY_NODE_BITS =
                DIRTY_ATTRIBUTES | DIRTY_PIPES | DIRTY_MEMBER |
                DIRTY_OBJECT_BITS
        };

        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_NODE_BITS; }

    private:
        /** Pipe children. */
        Pipes _pipes;

        /** The parent config. */
        C* const _config;

        struct BackupData
        {
            BackupData();
            /** Integer attributes. */
            int32_t iAttributes[IATTR_ALL];
        }
            _data, _backup;

        bool _isAppNode; //!< execute render tasks in application process

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class, class, class > friend class Pipe;
        void _addPipe( P* pipe );
        bool _removePipe( P* pipe );

        EQFABRIC_INL virtual uint32_t commitNB(); //!< @internal
        bool _mapNodeObjects() { return _config->mapNodeObjects(); }
    };

    template< class C, class N, class P, class V > EQFABRIC_INL
    std::ostream& operator << ( std::ostream&, const Node< C, N, P, V >& );
}
}

#endif // EQFABRIC_NODE_H

