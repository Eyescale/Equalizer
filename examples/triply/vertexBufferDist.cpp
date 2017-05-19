
/* Copyright (c) 2008-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "vertexBufferDist.h"

#include "vertexBufferLeaf.h"
#include "vertexBufferRoot.h"

namespace triply
{
VertexBufferDist::VertexBufferDist(VertexBufferRoot& root, co::NodePtr master,
                                   co::LocalNodePtr localNode,
                                   const eq::uint128_t& modelID)
    : VertexBufferDist(root, root, master, localNode, modelID)
{
}

VertexBufferDist::VertexBufferDist(VertexBufferRoot& root,
                                   VertexBufferBase& node, co::NodePtr master,
                                   co::LocalNodePtr localNode,
                                   const eq::uint128_t& modelID)
    : _root(root)
    , _node(node)
    , _changeType(STATIC)
{
    if (!localNode->mapObject(this, modelID, master, co::VERSION_FIRST))
        throw std::runtime_error("Mapping of ply node failed");
}

VertexBufferDist::VertexBufferDist(VertexBufferRoot& root,
                                   co::LocalNodePtr localNode,
                                   const co::Object::ChangeType type,
                                   const co::CompressorInfo& compressor)
    : VertexBufferDist(root, root, localNode, type, compressor)
{
}

VertexBufferDist::VertexBufferDist(VertexBufferRoot& root,
                                   VertexBufferBase& node,
                                   co::LocalNodePtr localNode,
                                   const co::Object::ChangeType type,
                                   const co::CompressorInfo& compressor)
    : _root(root)
    , _node(node)
    , _left(node.getLeft() ? new VertexBufferDist(root, *node.getLeft(),
                                                  localNode, type, compressor)
                           : nullptr)
    , _right(node.getRight() ? new VertexBufferDist(root, *node.getRight(),
                                                    localNode, type, compressor)
                             : nullptr)
    , _changeType(type)
    , _compressor(compressor == COMPRESSOR_AUTO ? co::Object::chooseCompressor()
                                                : compressor)
{
    if (!localNode->registerObject(this))
        throw std::runtime_error("Register of ply node failed");
}

VertexBufferDist::~VertexBufferDist()
{
    _left.reset();
    _right.reset();
    if (getLocalNode())
        getLocalNode()->releaseObject(this);
}

void VertexBufferDist::getInstanceData(co::DataOStream& os)
{
    if (_left)
        os << _left->getID() << _left->_node.getType();
    else
        os << eq::uint128_t() << Type::none;

    if (_right)
        os << _right->getID() << _right->_node.getType();
    else
        os << eq::uint128_t() << Type::none;

    os << _node._boundingSphere << _node._range;

    if (_isRoot())
    {
        const VertexBufferData& data = _root._data;
        os << data.vertices << data.colors << data.normals << data.indices
           << _root._name;
    }
    if (_node.getType() == Type::leaf)
    {
        const VertexBufferLeaf& leaf =
            dynamic_cast<const VertexBufferLeaf&>(_node);

        os << leaf._boundingBox[0] << leaf._boundingBox[1]
           << uint64_t(leaf._vertexStart) << uint64_t(leaf._indexStart)
           << uint64_t(leaf._indexLength) << leaf._vertexLength;
    }
}

void VertexBufferDist::applyInstanceData(co::DataIStream& is)
{
    const eq::uint128_t& leftID = is.read<eq::uint128_t>();
    const Type leftType = is.read<Type>();
    const eq::uint128_t& rightID = is.read<eq::uint128_t>();
    const Type rightType = is.read<Type>();

    is >> _node._boundingSphere >> _node._range;

    if (_isRoot())
    {
        VertexBufferData& data = _root._data;
        is >> data.vertices >> data.colors >> data.normals >> data.indices >>
            _root._name;
    }
    switch (_node.getType())
    {
    case Type::leaf:
    {
        VertexBufferLeaf& leaf = dynamic_cast<VertexBufferLeaf&>(_node);
        uint64_t i1, i2, i3;
        is >> leaf._boundingBox[0] >> leaf._boundingBox[1] >> i1 >> i2 >> i3 >>
            leaf._vertexLength;
        leaf._vertexStart = size_t(i1);
        leaf._indexStart = size_t(i2);
        leaf._indexLength = size_t(i3);
        return;
    }
    case Type::node:
        break;
    case Type::root:
        break;
    default:
        throw std::runtime_error("Internal error: unexpected node type " +
                                 std::to_string(unsigned(_node.getType())));
    }

    VertexBufferNode& node = dynamic_cast<VertexBufferNode&>(_node);
    node._left = _createNode(leftType);
    if (node._left)
        _left.reset(new VertexBufferDist(_root, *node._left, getMasterNode(),
                                         getLocalNode(), leftID));

    node._right = _createNode(rightType);
    if (node._right)
        _right.reset(new VertexBufferDist(_root, *node._right, getMasterNode(),
                                          getLocalNode(), rightID));
}

std::unique_ptr<VertexBufferBase> VertexBufferDist::_createNode(
    const Type type) const
{
    switch (type)
    {
    case Type::none:
        return nullptr;
    case Type::node:
        return std::unique_ptr<VertexBufferBase>(new VertexBufferNode);
    case Type::leaf:
        return std::unique_ptr<VertexBufferBase>(
            new VertexBufferLeaf(_root._data));
    default:
        throw std::runtime_error("Internal error: unexpected node type " +
                                 std::to_string(unsigned(type)));
    }
}
}
