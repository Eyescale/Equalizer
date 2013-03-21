
#include "rendererBase.h"

#include "glslShaders.h"

#include <eq/client/log.h>

namespace massVolVis
{

RendererBase::RendererBase()
    : _shadersPtr( new GLSLShaders() )
    , _glewContext( 0 )
{
}


bool RendererBase::_loadShaders( const std::string &vShader, const std::string &fShader )
{
    if( !_shadersPtr->loadShaders( vShader, fShader, glewGetContext( )))
    {
        LBERROR << "Can't load glsl shaders" << std::endl;
        return false;
    }

    EQLOG( eq::LOG_CUSTOM ) << "glsl shaders loaded" << std::endl;
    return true;
}


GLhandleARB RendererBase::_getShaderProgram() const
{
    return _shadersPtr->getProgram();
}


}
