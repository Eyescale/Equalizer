/*
 *  shader.h
 *  volume
 *
 *  Created by huebner on 19.12.05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _shader_h_
#define _shader_h_

namespace eqShader
{
    
    bool loadShaders( const std::string &v_file, const std::string &f_file, GLhandleARB &shader );
    
}
#endif
