
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_QT_PIPE_H
#define EQ_QT_PIPE_H

#include <eq/systemPipe.h> // base class

namespace eq
{
namespace qt
{
/** Handles a GPU/screen through Qt */
class Pipe : public SystemPipe
{
public:
    /** Create a new Qt pipe for the given eq::Pipe. @version 1.10 */
    explicit Pipe( eq::Pipe* parent ) : SystemPipe( parent ) {}

    /** Destroy the Qt Pipe. @version 1.10 */
    virtual ~Pipe() {}

    /** Initialize the GPU. @version 1.10 */
    EQ_API bool configInit() override;

    /** De-initialize the GPU. @version 1.10 */
    void configExit() override {}
};

}
}

#endif
