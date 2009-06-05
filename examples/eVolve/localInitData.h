
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EVOLVE_LOCALINITDATA_H
#define EVOLVE_LOCALINITDATA_H

#include "initData.h"

class FrameData;

namespace eVolve
{
    class LocalInitData : public InitData
    {
    public:
        LocalInitData();

        void parseArguments( int argc, char** argv );

        bool               isResident()     const { return _isResident; }
        bool               getOrtho()       const { return _ortho;  }
        uint32_t           getMaxFrames()   const { return _maxFrames; }

        const LocalInitData& operator = ( const LocalInitData& from );

    private:
        uint32_t    _maxFrames;
        bool        _isResident;
        bool        _ortho;
    };
}

#endif // EVOLVE_LOCALINITDATA_H
