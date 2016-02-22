
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "../channelListener.h" // nested base class

#include <lunchbox/hash.h>
#include <deque>
#include <map>

namespace eq
{
namespace server
{
    std::ostream& operator << ( std::ostream& os, const ViewEqualizer* );

    /**
     * An Equalizer allocating resources to multiple destination channels of a
     * single view.
     */
    class ViewEqualizer : public Equalizer
    {
    public:
        EQSERVER_API ViewEqualizer();
        ViewEqualizer( const ViewEqualizer& from );
        virtual ~ViewEqualizer();
        void toStream( std::ostream& os ) const final { os << this; }

        /** @sa Equalizer::attach. */
        void attach( Compound* compound ) final;

        /** @sa CompoundListener::notifyUpdatePre */
        void notifyUpdatePre( Compound* compound,
                              const uint32_t frameNumber ) final;

        uint32_t getType() const final { return fabric::VIEW_EQUALIZER; }

    protected:
        void notifyChildAdded( Compound*, Compound* ) override
            { LBASSERT( _listeners.empty( )); }
        void notifyChildRemove( Compound*, Compound* ) override
            { LBASSERT( _listeners.empty( )); }

    private:
        class Listener : public ChannelListener
        {
        public:
            Listener();
            virtual ~Listener();

            void update( Compound* compound );
            void clear();

            virtual void notifyLoadData( Channel* channel,
                                         uint32_t frameNumber,
                                         const Statistics& statistics,
                                         const Viewport& region );
            struct Load
            {
                static Load NONE;

                Load( const uint32_t frame_, const uint32_t missing_,
                      const int64_t time_ );
                bool operator == ( const Load& rhs ) const;

                uint32_t frame;
                uint32_t missing;
                uint32_t nResources;
                int64_t time;
            };

            /** @return the frame number of the youngest complete load. */
            uint32_t findYoungestLoad( const uint32_t frame ) const;
            /** Delete older loads and return the load belonging to the frame.*/
            const Load& useLoad( const uint32_t frameNumber );
            /** Insert a new, empty load for the given frame. */
            void newLoad( const uint32_t frameNumber, const uint32_t nChannels);
            /** @return the size of the history stash. */
            size_t getNLoads() const { return _loads.size(); }

        private:
            typedef lunchbox::PtrHash< Channel*, uint32_t > TaskIDHash;
            TaskIDHash _taskIDs;

            typedef std::deque< Load > LoadDeque;
            LoadDeque _loads;

            Load& _getLoad( const uint32_t frameNumber );
            friend std::ostream& operator << ( std::ostream& os,
                                               const ViewEqualizer::Listener& );
        };
        friend std::ostream& operator << ( std::ostream& os,
                                           const ViewEqualizer::Listener& );
        friend std::ostream& operator << ( std::ostream& os,
                                         const ViewEqualizer::Listener::Load& );

        typedef std::vector< Listener::Load > Loads;
        typedef std::vector< Listener > Listeners;
        /** Per-child listener gathering load data. */
        Listeners _listeners;

        /** The total number of available resources. */
        size_t _nPipes;

        /** Update channel load subscription. */
        void _updateListeners();
        /** Update resource count. */
        void _updateResources();
        /** Assign resources to children. */
        void _update( const uint32_t frameNumber );
        /** Find the frame number to use for update. */
        uint32_t _findInputFrameNumber() const;
    };
    std::ostream& operator << ( std::ostream& os,
                                const ViewEqualizer::Listener& listener );
    std::ostream& operator << ( std::ostream& os,
                                const ViewEqualizer::Listener::Load& load );
}
}

#endif // EQS_VIEWEQUALIZER_H
