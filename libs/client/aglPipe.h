
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
                      2009, Maxim Makhinya
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

#ifndef EQ_AGL_PIPE_H
#define EQ_AGL_PIPE_H

#include <eq/systemPipe.h> // base class
#include <eq/os.h>     // CG types

namespace eq
{
    /**
     * Equalizer default implementation to handle an AGL GPU.
     *
     * When using AGLWindow as a system window for any window of a Pipe, the
     * pipe needs to have an AGLPipe as its SystemPipe.
     */
    class AGLPipe : public SystemPipe
    {
    public:
        /** Create a new AGL pipe for the given eq::Pipe. @version 1.0 */
        AGLPipe( Pipe* parent );

        /** Destroy the AGL pipe. @version 1.0 */
        virtual ~AGLPipe( );

        /** @name AGL initialization */
        //@{
        /** 
         * Initialize this pipe for the AGL window system.
         * 
         * @return true if the initialization was successful, false otherwise.
         * @version 1.0
         */
        virtual bool configInit( );

        /** 
         * De-initialize this pipe for the AGL window system.
         * 
         * @return true if the deinitialization was successful, false otherwise.
         * @version 1.0
         */
        virtual void configExit( );
        //@}

        /** @return the CG display ID for this pipe. @version 1.0 */
        CGDirectDisplayID getCGDisplayID() const { return _cgDisplayID; }

    private:

        /** @name Data Access. */
        //@{
        /** 
         * Set the CG display ID for this pipe.
         * 
         * This function should only be called from configInit() or
         * configExit().
         *
         * @param id the CG display ID for this pipe.
         */
        void _setCGDisplayID( CGDirectDisplayID id );
        //@}

        /** Window-system specific display information. */
        CGDirectDisplayID _cgDisplayID;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}

#endif // EQ_AGL_PIPE_H
