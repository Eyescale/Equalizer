
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "error.h"

#include <eq/fabric/errorRegistry.h>
#include <eq/fabric/global.h>

namespace eVolve
{
namespace
{
struct ErrorData
{
    const uint32_t code;
    const std::string text;
};

ErrorData _errors[] = {
    {ERROR_EVOLVE_ARB_SHADER_OBJECTS_MISSING,
     "GL_ARB_shader_objects extension missing"},
    {ERROR_EVOLVE_EXT_BLEND_FUNC_SEPARATE_MISSING,
     "GL_ARB_shader_objects extension missing"},
    {ERROR_EVOLVE_ARB_MULTITEXTURE_MISSING,
     "GL_ARB_shader_objects extension missing"},
    {ERROR_EVOLVE_LOADSHADERS_FAILED, "Can't load shaders"},
    {ERROR_EVOLVE_LOADMODEL_FAILED, "Can't load model"},
    {ERROR_EVOLVE_MAPOBJECT_FAILED,
     "Mapping data from application process failed"},

    {0, ""} // last!
};
}

void initErrors()
{
    eq::fabric::ErrorRegistry& registry =
        eq::fabric::Global::getErrorRegistry();

    for (size_t i = 0; _errors[i].code != 0; ++i)
        registry.setString(_errors[i].code, _errors[i].text);
}

void exitErrors()
{
    eq::fabric::ErrorRegistry& registry =
        eq::fabric::Global::getErrorRegistry();

    for (size_t i = 0; _errors[i].code != 0; ++i)
        registry.eraseString(_errors[i].code);
}
}
