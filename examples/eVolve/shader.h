/* Copyright (c) 2005       Thomas Huebner
                 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_SHADER_H
#define EVOLVE_SHADER_H

namespace eqShader
{
    bool loadShaders( const std::string &vShader, const std::string &fShader,
                      GLhandleARB &shader );
    
}
#endif // EVOLVE_SHADER_H
