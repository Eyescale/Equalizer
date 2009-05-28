
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
#include "../channelListener.h" // base class

#include <eq/client/types.h>
#include <eq/base/hash.h>
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

    protected:        
        // override in sub-classes to handle dynamic compounds.
        virtual void notifyChildAdded( Compound* compound, Compound* child )
            { EQASSERT( _listeners.empty( )); }
        virtual void notifyChildRemove( Compound* compound, Compound* child )
            { EQASSERT( _listeners.empty( )); }

    private:
        class Listener : public ChannelListener
        {
        public:
            Listener();
            virtual ~Listener();

            void update( Compound* compound );
            void clear();

            virtual void notifyLoadData( Channel* channel, 
                                         const uint32_t frameNumber,
                                         const uint32_t nStatistics,
                                         const eq::Statistic* statistics );

        private:
            typedef base::PtrHash< Channel*, uint32_t > TaskIDHash;
            TaskIDHash _taskIDs;
        };

        typedef std::vector< Listener > ListenerVector;
        ListenerVector _listeners;

        void _updateListeners();
    };
}
}

#endif // EQS_VIEWEQUALIZER_H
