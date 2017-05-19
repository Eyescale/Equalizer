
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EVOLVE_ERROR_H
#define EVOLVE_ERROR_H

#include <eq/eq.h>

namespace eVolve
{
/** Defines errors produced by eVolve. */
enum Error
{
    ERROR_EVOLVE_ARB_SHADER_OBJECTS_MISSING = eq::ERROR_CUSTOM,
    ERROR_EVOLVE_EXT_BLEND_FUNC_SEPARATE_MISSING,
    ERROR_EVOLVE_ARB_MULTITEXTURE_MISSING,
    ERROR_EVOLVE_LOADSHADERS_FAILED,
    ERROR_EVOLVE_LOADMODEL_FAILED,
    ERROR_EVOLVE_MAPOBJECT_FAILED
};

/** Set up eVolve-specific error codes. */
void initErrors();

/** Clear eVolve-specific error codes. */
void exitErrors();
}
#endif // EVOLVE_ERROR_H
