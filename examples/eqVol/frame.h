
#ifndef EQ_VOL_FRAME_H
#define EQ_VOL_FRAME_H

#include <eq/eq.h>

namespace eqVol
{
	using namespace eq;
	

    class EQ_EXPORT FrameVol : public eq::Frame
    {
    public:
		void syncAssemble() 
		{
		    //_getData()->syncAssemble();
		}        
    };
};
#endif // EQ_VOL_FRAME_H
