
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include "typedefs.h"

#include <eq/eq.h>


namespace eqPly
{
    class FrameData;
    class InitData;

    static const uint32_t primeNumberTable[100] = {
                    739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
                    811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
                    877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
                    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
                    1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
                    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
                    1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
                    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
                    1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
                    1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451
                    };

    /**
     * The rendering entity, updating a part of a Window.
     */
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent );

        bool stopRendering() const;

    protected:
        virtual ~Channel() {}

        virtual bool configInit( const eq::uint128_t& initID );
        virtual bool configExit();
        virtual void frameClear( const eq::uint128_t& frameID );
        virtual void frameDraw( const eq::uint128_t& frameID );
        virtual void frameAssemble( const eq::uint128_t& frameID );
        virtual void frameReadback( const eq::uint128_t& frameID );
        virtual void frameStart( const eq::uint128_t& frameID,
                                 const uint32_t frameNumber );
        virtual void frameFinish( const eq::uint128_t& frameID,
                                  const uint32_t frameNumber );
        virtual void frameViewStart( const eq::uint128_t& frameID );
        virtual void frameViewFinish( const eq::uint128_t& frameID );

        virtual bool useOrtho() const;
        virtual eq::Vector2f getJitter() const;

        virtual void notifyStopFrame( const uint32_t lastFrameNumber )
            { _frameStartRendering = lastFrameNumber + 1; }

    private:
        void _drawModel( const Model* model );
        void _drawOverlay();
        void _drawHelp();
        void _updateNearFar( const mesh::BoundingSphere& boundingSphere );
        void _initFrustum( eq::FrustumCullerf& frustum, 
                           const mesh::BoundingSphere& boundingSphere );

        bool _isDone() const;

        void _initJitter();
        bool _initAccum();

        /** the subpixel for this step. */
        eq::Vector2i _getJitterStep() const;

        const FrameData& _getFrameData() const;
        const Model*     _getModel();

        const Model* _model;
        eq::uint128_t _modelID;
        uint32_t _frameStartRendering;

        struct Accum
        {
            Accum() : buffer( 0 ), step( 0 ), stepsDone( 0 ), transfer( false )
                {}

            eq::util::Accum* buffer;
            int32_t step;
            uint32_t stepsDone;
            bool transfer;
        }
        _accum[ eq::NUM_EYES ];

        eq::PixelViewport _currentPVP;
    };
}



#endif // EQ_PLY_CHANNEL_H

