
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

bool Config::init( const uint32_t initID )
{
    _running = eq::Config::init( initID );
    return _running;
}

bool Config::handleEvent( eq::ConfigEvent* event )
{
    switch( event->type )
    {
        case eq::ConfigEvent::TYPE_KEY_PRESS:
            switch( event->keyPress.key )
            {
                case 'q':
                case eq::KC_ESCAPE:
                    _running = false;
                    return true;
            }
            break;

        case eq::ConfigEvent::TYPE_MOUSE_MOTION:
            
        default:
            break;
    }
    return eq::Config::handleEvent( event );
}
