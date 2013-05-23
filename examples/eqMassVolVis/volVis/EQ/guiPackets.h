
/* Copyright (c) 2011, Fatih Erol
 *               2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */


#ifndef MASS_VOL__GUI_PACKETS_H
#define MASS_VOL__GUI_PACKETS_H

#include "guiConnectionDefs.h"

#include <co/commands.h>
#include <msv/types/types.h>


namespace massVolVis
{

enum GUICommand
{
    CMD_GUI_SETX     = co::CMD_NODE_CUSTOM,
    CMD_GUI_SET_FILE,
    CMD_GUI_SET_TF
};

}// namespace massVolVis


#endif  // MASS_VOL__GUI_PACKETS_H
