
/* Copyright (c) 2007-2017, Tobias Wolf <twolf@access.unizh.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
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

#include "vertexBufferNode.h"
#include "vertexBufferLeaf.h"
#include "vertexBufferState.h"
#include "vertexData.h"
#include <set>

namespace triply
{
inline static bool _subdivide(const Index length, const size_t depth)
{
    return (length > LEAF_SIZE) || (depth < 3 && length > 1);
}

/*  Continue kd-tree setup, create intermediary or leaf nodes as required.  */
void VertexBufferNode::setupTree(VertexData& data, const Index start,
                                 const Index length, const Axis axis,
                                 const size_t depth,
                                 VertexBufferData& globalData,
                                 boost::progress_display& progress)
{
    data.sort(start, length, axis);
    const Index median = start + (length / 2);

    // left child will include elements smaller than the median
    const Index leftLength = length / 2;
    const bool subdivideLeft = _subdivide(leftLength, depth);

    if (subdivideLeft)
        _left.reset(new VertexBufferNode);
    else
        _left.reset(new VertexBufferLeaf(globalData));

    // right child will include elements equal to or greater than the median
    const Index rightLength = (length + 1) / 2;
    const bool subdivideRight = _subdivide(rightLength, depth);

    if (subdivideRight)
        _right.reset(new VertexBufferNode);
    else
        _right.reset(new VertexBufferLeaf(globalData));

    // move to next axis and continue contruction in the child nodes
    const Axis newAxisLeft =
        subdivideLeft ? data.getLongestAxis(start, leftLength) : AXIS_X;
    const Axis newAxisRight =
        subdivideRight ? data.getLongestAxis(median, rightLength) : AXIS_X;

    _left->setupTree(data, start, leftLength, newAxisLeft, depth + 1,
                     globalData, progress);
    _right->setupTree(data, median, rightLength, newAxisRight, depth + 1,
                      globalData, progress);
    if (depth == 3)
        ++progress;
}

void VertexBufferNode::updateBounds()
{
    _left->updateBounds();
    _right->updateBounds();

    // compute enclosing box
    const auto& box1 = _left->getBoundingBox();
    const auto& box2 = _right->getBoundingBox();
    _boundingBox[0][0] = std::min(box1[0][0], box2[0][0]);
    _boundingBox[0][1] = std::min(box1[0][1], box2[0][1]);
    _boundingBox[0][2] = std::min(box1[0][2], box2[0][2]);
    _boundingBox[1][0] = std::max(box1[1][0], box2[1][0]);
    _boundingBox[1][1] = std::max(box1[1][1], box2[1][1]);
    _boundingBox[1][2] = std::max(box1[1][2], box2[1][2]);

    // compute enclosing sphere
    const auto sphere1 = _left->getBoundingSphere();
    const auto sphere2 = _right->getBoundingSphere();
    const Vertex center1(sphere1.array);
    const Vertex center2(sphere2.array);
    Vertex c1ToC2 = center2 - center1;
    c1ToC2.normalize();

    const Vertex outer1 = center1 - c1ToC2 * sphere1.w();
    const Vertex outer2 = center2 + c1ToC2 * sphere2.w();

    Vertex vertexBoundingSphere = Vertex(outer1 + outer2) * 0.5f;
    _boundingSphere.x() = vertexBoundingSphere.x();
    _boundingSphere.y() = vertexBoundingSphere.y();
    _boundingSphere.z() = vertexBoundingSphere.z();
    _boundingSphere.w() = Vertex(outer1 - outer2).length() * 0.5f;
}

/*  Compute the range from the children's ranges.  */
void VertexBufferNode::updateRange()
{
    _left->updateRange();
    _right->updateRange();

    // set node range to min/max of the children's ranges
    _range[0] = std::min(_left->getRange()[0], _right->getRange()[0]);
    _range[1] = std::max(_left->getRange()[1], _right->getRange()[1]);
}

/*  Draw the node by rendering the children.  */
void VertexBufferNode::draw(VertexBufferState& state) const
{
    if (state.stopRendering())
        return;

    _left->draw(state);
    _right->draw(state);
}

/*  Read node from memory and continue with remaining nodes.  */
void VertexBufferNode::fromMemory(char** addr, VertexBufferData& globalData)
{
    // read node itself
    Type nodeType;
    memRead(reinterpret_cast<char*>(&nodeType), addr, sizeof(nodeType));
    if (nodeType != Type::node)
        throw MeshException("Error reading binary file. Expected a node, got " +
                            std::to_string(unsigned(nodeType)));
    VertexBufferBase::fromMemory(addr, globalData);

    // read left child (peek ahead)
    memRead(reinterpret_cast<char*>(&nodeType), addr, sizeof(nodeType));
    if (nodeType != Type::node && nodeType != Type::leaf)
        throw MeshException(
            "Error reading binary file. Expected either a "
            "regular or a leaf node, but found neither.");
    *addr -= sizeof(nodeType);
    if (nodeType == Type::node)
        _left.reset(new VertexBufferNode);
    else
        _left.reset(new VertexBufferLeaf(globalData));
    _left->fromMemory(addr, globalData);

    // read right child (peek ahead)
    memRead(reinterpret_cast<char*>(&nodeType), addr, sizeof(nodeType));
    if (nodeType != Type::node && nodeType != Type::leaf)
        throw MeshException(
            "Error reading binary file. Expected either a "
            "regular or a leaf node, but found neither.");
    *addr -= sizeof(nodeType);
    if (nodeType == Type::node)
        _right.reset(new VertexBufferNode);
    else
        _right.reset(new VertexBufferLeaf(globalData));
    _right->fromMemory(addr, globalData);
}

/*  Write node to output stream and continue with remaining nodes.  */
void VertexBufferNode::toStream(std::ostream& os)
{
    const Type nodeType = Type::node;
    os.write(reinterpret_cast<const char*>(&nodeType), sizeof(nodeType));
    VertexBufferBase::toStream(os);
    _left->toStream(os);
    _right->toStream(os);
}
}
