
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef COBASE_MEMCPY_H
#define COBASE_MEMCPY_H

#include <eq/base/base.h>

namespace co
{
namespace base
{
/** @internal */
inline void fastCopy( void *dst, const void *src, uint32_t nbytes )
{
#ifdef _WIN64
    ::memcpy(dst,src,nbytes);
#elif defined WIN32 
    static const int BLOCK_SIZE = 64;
    
    long bytesDiff;
    if (nbytes < BLOCK_SIZE )
    {
        ::memcpy(dst,src,nbytes);
        return;
    }
    /* each iteration copy 64 bytes */
    _asm 
    {
        mov esi, src 
        mov edi, dst 
        mov ecx, nbytes 
        shr ecx, 6 
        mov bytesDiff, ecx  // for know how much bytes we have to copy at 
                            // the end 
        shl bytesDiff, 6
            
        loop1: 
            movq mm1,  0[ESI] 
            movq mm2,  8[ESI] 
            movq mm3, 16[ESI] 
            movq mm4, 24[ESI] 
            movq mm5, 32[ESI] 
            movq mm6, 40[ESI] 
            movq mm7, 48[ESI] 
            movq mm0, 56[ESI] 
            
            movntq  0[EDI], mm1 
            movntq  8[EDI], mm2 
            movntq 16[EDI], mm3 
            movntq 24[EDI], mm4 
            movntq 32[EDI], mm5 
            movntq 40[EDI], mm6 
            movntq 48[EDI], mm7 
            movntq 56[EDI], mm0 
            
            add esi, 64 
            add edi, 64 
            dec ecx 
            jnz loop1 
            
            emms 
    }


    if (nbytes - bytesDiff == 0 )
        return;
    
    if (nbytes - bytesDiff < BLOCK_SIZE )
    {
        const char* src8 = reinterpret_cast<const char*>( src );
        char* dst8       = reinterpret_cast<char*>( dst );
        ::memcpy( dst8 + bytesDiff, src8 + bytesDiff, nbytes - bytesDiff );
    }
#else // !WIN32

    ::memcpy(dst,src,nbytes);

#endif
}

}
}
#endif //COBASE_FILESEARCH_H
