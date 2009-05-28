
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQS_VIEWEQUALIZER_H
#define EQS_VIEWEQUALIZER_H

#include "equalizer.h"          // base class

#include <eq/client/types.h>
#include <deque>
#include <map>

namespace eq
{
namespace server
{
    class Compound;
    class ViewEqualizer;
    std::ostream& operator << ( std::ostream& os, const ViewEqualizer* );

    /** Destination-driven scaling.*/
    class ViewEqualizer : public Equalizer
    {
    public:            
        ViewEqualizer();
        ViewEqualizer( const ViewEqualizer& from );
        virtual ~ViewEqualizer();
        virtual Equalizer* clone() const { return new ViewEqualizer(*this); }
        virtual void toStream( std::ostream& os ) const { os << this; }
            
        /** @sa Equalizer::attach. */
        virtual void attach( Compound* compound );
        
        /** @sa CompoundListener::notifyUpdatePre */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber );
        
    private:
    };
}
}

#endif // EQS_VIEWEQUALIZER_H
